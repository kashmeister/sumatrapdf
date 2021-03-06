/* Copyright 2014 the SumatraPDF project authors (see AUTHORS file).
   License: GPLv3 */

#include "BaseUtil.h"
#include "Doc.h"

#include "BaseEngine.h"
#include "EbookDoc.h"
#include "MobiDoc.h"

Doc::Doc(const Doc& other)
{
    Clear();
    type = other.type;
    generic = other.generic;
    error = other.error;
    filePath.Set(str::Dup(other.filePath));
}

Doc& Doc::operator=(const Doc& other)
{
    if (this != &other) {
        type = other.type;
        generic = other.generic;
        error = other.error;
        filePath.Set(str::Dup(other.filePath));
    }
    return *this;
}

Doc::~Doc()
{
}

// delete underlying object
void Doc::Delete()
{
    switch (type) {
    case Doc_Epub:
        delete epubDoc;
        break;
    case Doc_Fb2:
        delete fb2Doc;
        break;
    case Doc_Mobi:
        delete mobiDoc;
        break;
    case Doc_MobiTest:
        delete mobiTestDoc;
        break;
    case Doc_None:
        break;
    default:
        CrashIf(!IsEngine());
        delete engine;
        break;
    }

    Clear();
}

Doc::Doc(BaseEngine *doc, DocType engineType)
{
    Clear();
    type = doc ? engineType : Doc_None;
    engine = doc;
    CrashIf(doc && !IsEngine());
}

Doc::Doc(EpubDoc *doc)
{
    Clear();
    type = doc ? Doc_Epub : Doc_None;
    epubDoc = doc;
}

Doc::Doc(Fb2Doc *doc)
{
    Clear();
    type = doc ? Doc_Fb2 : Doc_None;
    fb2Doc = doc;
}

Doc::Doc(MobiDoc *doc)
{
    Clear();
    type = doc ? Doc_Mobi : Doc_None;
    mobiDoc = doc;
}

Doc::Doc(MobiTestDoc *doc)
{ 
    Clear();
    type = doc ? Doc_MobiTest : Doc_None;
    mobiTestDoc = doc;
}

void Doc::Clear()
{
    type = Doc_None;
    generic = NULL;
    error = Error_None;
    filePath.Set(NULL);
}

BaseEngine *Doc::AsEngine() const
{
    if (IsEngine())
        return engine;
    return NULL;
}

MobiDoc *Doc::AsMobi() const
{
    if (Doc_Mobi == type)
        return mobiDoc;
    return NULL;
}

MobiTestDoc *Doc::AsMobiTest() const
{
    if (Doc_MobiTest == type)
        return mobiTestDoc;
    return NULL;
}

EpubDoc *Doc::AsEpub() const
{
    if (Doc_Epub == type)
        return epubDoc;
    return NULL;
}

Fb2Doc *Doc::AsFb2() const
{
    if (Doc_Fb2 == type)
        return fb2Doc;
    return NULL;
}

// return true if this is document supported by ebook UI
bool Doc::IsEbook() const
{
    switch (type) {
    case Doc_Epub:
    case Doc_Fb2:
    case Doc_Mobi:
    case Doc_MobiTest:
        return true;
    default:
        CrashIf(!IsNone() && !IsEngine());
        return false;
    }
}

bool Doc::IsEngine() const
{
    if ((type & Doc_BaseEngine)) {
        return true;
    }
    CrashIf(!IsNone() && !IsEbook());
    return false;
}

// the caller should make sure there is a document object
const WCHAR *Doc::GetFilePathFromDoc() const
{
    switch (type) {
    case Doc_Epub:
        return epubDoc->GetFileName();
    case Doc_Fb2:
        return fb2Doc->GetFileName();
    case Doc_Mobi:
        return mobiDoc->GetFileName();
    case Doc_MobiTest:
        return NULL;
    case Doc_None:
        return NULL;
    default:
        CrashIf(!IsEngine());
        return engine->FileName();
    }
}

const WCHAR *Doc::GetFilePath() const
{
    if (filePath) {
        // verify it's consistent with the path in the doc
        const WCHAR *docPath = GetFilePathFromDoc();
        CrashIf(docPath && !str::Eq(filePath, docPath));
        return filePath;
    }
    CrashIf(!generic && !IsNone());
    return GetFilePathFromDoc();
}

WCHAR *Doc::GetProperty(DocumentProperty prop)
{
    switch (type) {
    case Doc_Epub:
        return epubDoc->GetProperty(prop);
    case Doc_Fb2:
        return fb2Doc->GetProperty(prop);
    case Doc_Mobi:
        return mobiDoc->GetProperty(prop);
    case Doc_MobiTest:
        return NULL;
    case Doc_None:
        return NULL;
    default:
        CrashIf(!IsEngine());
        return engine->GetProperty(prop);
    }
}

const char *Doc::GetHtmlData(size_t &len)
{
    switch (type) {
    case Doc_Epub:
        return epubDoc->GetTextData(&len);
    case Doc_Fb2:
        return fb2Doc->GetTextData(&len);
    case Doc_Mobi:
        return mobiDoc->GetBookHtmlData(len);
    case Doc_MobiTest:
        return mobiTestDoc->GetBookHtmlData(len);
    default:
        CrashIf(true);
        return NULL;
    }
}

size_t Doc::GetHtmlDataSize()
{
    switch (type) {
    case Doc_Epub:
        return epubDoc->GetTextDataSize();
    case Doc_Fb2:
        return fb2Doc->GetTextDataSize();
    case Doc_Mobi:
        return mobiDoc->GetBookHtmlSize();
    case Doc_MobiTest:
        return mobiTestDoc->GetBookHtmlSize();
    default:
        CrashIf(true);
        return NULL;
    }
}

ImageData *Doc::GetCoverImage()
{
    switch (type) {
    case Doc_Fb2:
        return fb2Doc->GetCoverImage();
    case Doc_Mobi:
        return mobiDoc->GetCoverImage();
    default:
        return NULL;
    }
}

Doc Doc::CreateFromFile(const WCHAR *filePath)
{
    Doc doc;
    if (EpubDoc::IsSupportedFile(filePath))
        doc = Doc(EpubDoc::CreateFromFile(filePath));
    else if (Fb2Doc::IsSupportedFile(filePath))
        doc = Doc(Fb2Doc::CreateFromFile(filePath));
    else if (MobiDoc::IsSupportedFile(filePath))
        doc = Doc(MobiDoc::CreateFromFile(filePath));

    // if failed to load and more specific error message hasn't been
    // set above, set a generic error message
    if (doc.IsNone()) {
        CrashIf(doc.error);
        doc.error = Error_Unknown;
        doc.filePath.Set(str::Dup(filePath));
    }
    else {
        CrashIf(!Doc::IsSupportedFile(filePath));
    }
    CrashIf(!doc.generic && !doc.IsNone());
    return doc;
}

bool Doc::IsSupportedFile(const WCHAR *filePath, bool sniff)
{
    return EpubDoc::IsSupportedFile(filePath, sniff) ||
           Fb2Doc::IsSupportedFile(filePath, sniff) ||
           MobiDoc::IsSupportedFile(filePath, sniff);
}
