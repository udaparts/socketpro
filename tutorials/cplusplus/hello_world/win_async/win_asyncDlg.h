#pragma once

class Cwin_asyncDlg : public CDialogEx {
    // Construction
public:
    Cwin_asyncDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data

    enum {
        IDD = IDD_WIN_ASYNC_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedTestButton();

    DECLARE_MESSAGE_MAP()

private:
    //resumable functions in C++
    CAwTask ExecuteTask();
    CAwTask ExecuteTasksInBatch();

private:
    CSocketPool<HelloWorld> m_spHw;
};
