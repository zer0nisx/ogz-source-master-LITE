// WorldEditDoc.h - ImGui version (no MFC)
#pragma once

#include "RNameSpace.h"
#include "RBspObject.h"
#include <memory>
#include <string>

_USING_NAMESPACE_REALSPACE2

class WorldEditDoc
{
public:
    WorldEditDoc();
    ~WorldEditDoc();

    bool OpenDocument(const wchar_t* lpszPathName);
    void CloseDocument();
    bool IsDocumentOpen() const { return m_pBspObject != nullptr; }

    std::unique_ptr<RBspObject> m_pBspObject;
    bool m_bLastPicked = false;
    RBSPPICKINFO m_LastPicked;

    std::wstring GetFilePath() const { return m_filePath; }

private:
    std::wstring m_filePath;
};
