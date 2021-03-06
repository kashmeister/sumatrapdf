/* Copyright 2014 the SumatraPDF project authors (see AUTHORS file).
   License: GPLv3 */

#ifndef EngineManager_h
#define EngineManager_h

enum EngineType {
    Engine_None = 0,
    // the EngineManager tries to create a new engine
    // in the following order (types on the same line
    // share common code and reside in the same file)
    Engine_PDF, Engine_XPS,
    Engine_DjVu,
    Engine_Image, Engine_ImageDir, Engine_ComicBook,
    Engine_PS,
    Engine_Chm,
    Engine_Epub, Engine_Fb2, Engine_Mobi, Engine_Pdb, Engine_Chm2, Engine_Tcr, Engine_Html, Engine_Txt,
};

class BaseEngine;
class PasswordUI;

namespace EngineManager {

bool IsSupportedFile(const WCHAR *filePath, bool sniff=false, bool enableEbookEngines=true);
BaseEngine *CreateEngine(const WCHAR *filePath, PasswordUI *pwdUI=NULL, EngineType *typeOut=NULL, bool useAlternateChmEngine=false, bool enableEbookEngines=true);

inline BaseEngine *CreateEngine(const WCHAR *filePath, bool useAlternateChmEngine) {
    return CreateEngine(filePath, NULL, NULL, useAlternateChmEngine, true);
}

}

#endif
