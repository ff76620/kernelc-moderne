#pragma once
#include 
#include "../ui/dialog_manager.hpp"

namespace Win32Compat {
  class ComDlg32Wrapper {
  private:
    DialogManager* dialogManager;

  public:
    ComDlg32Wrapper() {
      dialogManager = DialogManager::getInstance();
    }

    BOOL GetOpenFileNameW(LOPENFILENAMEW ofn) {
      DialogParams params;
      params.title = ofn->lpstrTitle;
      params.filter = ofn->lpstrFilter;
      params.initialDir = ofn->lpstrInitialDir;
      return dialogManager->showOpenDialog(params, ofn->lpstrFile);
    }

    BOOL GetSaveFileNameW(LOPENFILENAMEW ofn) {
      DialogParams params;
      params.title = ofn->lpstrTitle; 
      params.filter = ofn->lpstrFilter;
      params.initialDir = ofn->lpstrInitialDir;
      return dialogManager->showSaveDialog(params, ofn->lpstrFile);
    }
  };
}