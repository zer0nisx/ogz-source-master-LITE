// WorldEditDoc.cpp - ImGui version (no MFC)
#include "stdafx.h"
#include "WorldEditDoc.h"
#include "RBspObject.h"
#include "FileInfo.h"
#include "RMaterialList.h"
#include "MDebug.h"

WorldEditDoc::WorldEditDoc()
{
}

WorldEditDoc::~WorldEditDoc()
{
    CloseDocument();
}

bool WorldEditDoc::OpenDocument(const wchar_t* lpszPathName)
{
    CloseDocument();

    m_pBspObject = std::make_unique<RBspObject>();
    
    // Convert wide string to narrow string
    char narrowPath[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, lpszPathName, -1, narrowPath, MAX_PATH, nullptr, nullptr);
    
    if (!m_pBspObject->Open(narrowPath, RBspObject::ROpenMode::Editor))
    {
        m_pBspObject.reset();
        mlog("Failed to open map: %s\n", narrowPath);
        return false;
    }

    m_bLastPicked = false;
    m_filePath = lpszPathName;
    return true;
}

void WorldEditDoc::CloseDocument()
{
    m_pBspObject.reset();
    m_bLastPicked = false;
    m_filePath.clear();
}
