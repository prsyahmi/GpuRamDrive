#include "stdafx.h"
#include "DataGrid.h"


//http://www.codeguru.com/cpp/controls/controls/gridcontrol/article.php/c10319/CDataGrid-Control.htm
//http://www.codeproject.com/KB/miscctrl/DataGridControl.aspx
/* DataGrid global variables */
DG_LIST* g_DGList = NULL;
int g_DGGridNumber = 0;
static TCHAR szClassName[] = _T("DATAGRID");

//Original Edit procedure
WNDPROC EditProc;

/*
INT_PTR CDialogListTreat::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
INT_PTR CMainWindow::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        if (HIWORD(wParam) == DGM_ITEMTEXTCHANGED)
            int iIndex=-1;
        break;
*/

CDataGrid::CDataGrid()
{
    // Init DataGrid variables
    m_hWnd = NULL;
    m_hParentWnd = NULL;
}


CDataGrid::~CDataGrid()
{
}


void CDataGrid::Resize()
{
    // Recalculate DataGrid relative size
    RECT rectParent;
    GetWindowRect(m_hParentWnd, &rectParent);
    SetWindowPos(m_hWnd, HWND_NOTOPMOST, int((double(m_DGRect.left) / 100.0) * (rectParent.right - rectParent.left)), int((double(m_DGRect.top) / 100.0) * (rectParent.bottom - rectParent.top)),
        int((double(m_DGRect.right) / 100.0) * (rectParent.right - rectParent.left)), int((double(m_DGRect.bottom) / 100.0) * (rectParent.bottom - rectParent.top)), SWP_NOZORDER | SWP_SHOWWINDOW);
    RECT clientRect;
    GetClientRect(m_hWnd, &clientRect);
    RecalcWindow(m_hWnd);
    InvalidateRect(m_hWnd, NULL, TRUE);
    UpdateWindow(m_hWnd);
}


BOOL CDataGrid::Create(RECT wndRect, HWND hParent, int numCols)
{
    BOOL result = FALSE;

    // Set DatGrid parent window handle
    m_hParentWnd = hParent;

    m_DGRect.left = wndRect.left;
    m_DGRect.top = wndRect.top;
    m_DGRect.right = wndRect.right;
    m_DGRect.bottom = wndRect.bottom;

    WNDCLASSEX wincl;
    wincl.hInstance = GetModuleHandle(NULL);
    wincl.lpszClassName = szClassName;//"DATAGRID";
    wincl.lpfnWndProc = DataGridProc;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    ATOM reg = RegisterClassEx(&wincl);
    DWORD error = GetLastError();

    // Register DataGrid window class
    if ((reg) || (error == ERROR_CLASS_ALREADY_EXISTS))
    {
        // Create DataGrid window
        m_hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, szClassName, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_VSCROLL,
            wndRect.left, wndRect.top, wndRect.right - wndRect.left, wndRect.bottom - wndRect.top, hParent, NULL, GetModuleHandle(NULL), NULL);

        ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (long)this);
        // Hide DataGrid window scroll bars
        //http://www.codeguru.com/cpp/controls/controls/gridcontrol/article.php/c10319/CDataGrid-Control.htm
        //To true?
              //ShowScrollBar( m_hWnd, SB_BOTH, FALSE );

        if (m_hWnd)
        {
            AddDGGrid(m_hWnd, m_hParentWnd);
            result = TRUE;
        }

        // Init DataGrid GDI objects
        InitDGGlobals(hParent, m_hWnd);

        DG_LIST* dgList = GetDGGrid(m_hWnd);
        if (dgList != NULL)
        {
            // Init DataGrid columns info
            if (numCols < DG_MAXCOLUMN)
                dgList->dg_ColumnNumber = numCols;
            else
                dgList->dg_ColumnNumber = DG_MAXCOLUMN;

            dgList->dg_Columns = new DG_COLUMN[dgList->dg_ColumnNumber];
            for (int i = 0; i < dgList->dg_ColumnNumber; i++)
            {
                dgList->dg_Columns[i].columnWidth = 50;
                dgList->dg_Columns[i].textAlign = DGTA_LEFT;
                _tcscpy(dgList->dg_Columns[i].columnText, _T(" "));
            }

            // Create EDIT control
            m_hEditWnd = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_CLIPSIBLINGS | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN, 0, 0, 100, 20, m_hWnd, NULL, GetModuleHandle(NULL), NULL);
            if (::IsWindow(m_hEditWnd) == FALSE) { // Hat es geklappt?
                // Nein? -> Fehler anzeigen!
                ::MessageBox(m_hWnd, _T("Can not create Edit!"), _T("Error!"), MB_OK | MB_ICONERROR);
                ::DestroyWindow(m_hWnd); // Fenster zerst?ren
                return 0;
            }


            SendMessage(m_hEditWnd, WM_SETFONT, (WPARAM)dgList->dg_hRowFont, MAKELPARAM(TRUE, 0));

            EditProc = (WNDPROC)GetWindowLongPtr(m_hEditWnd, GWLP_WNDPROC);
            //SendMessage (m_hEditWnd ,EM_LIMITTEXT,-1,0);
            // ::SetWindowLongPtr(m_hEditWnd, GWLP_WNDPROC, (long)SubEditProc);
            // ::SetWindowLong(m_hEditWnd, GWL_USERDATA, (long)dgList->dg_hEditWnd);
            dgList->dg_hEditWnd = m_hEditWnd;
        }

        // DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            HDC hDC = GetDC(dgList->dg_hWnd);
            RECT rectClient;
            GetClientRect(dgList->dg_hWnd, &rectClient);
            dgList->dg_hMemBitmap = CreateCompatibleBitmap(hDC, (rectClient.right - rectClient.left), (rectClient.bottom - rectClient.top));
            dgList->dg_hMemDC = CreateCompatibleDC(hDC);
            dgList->dg_hOldMemBitmap = (HBITMAP)SelectObject(dgList->dg_hMemDC, dgList->dg_hMemBitmap);
            SetFocus(dgList->dg_hWnd);
            ReleaseDC(dgList->dg_hWnd, hDC);
        }


    }

    return result;
}


BOOL CDataGrid::ResetGridColumns(int numNewCols)
{
    BOOL result = FALSE;
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Delete columns
        if (dgList->dg_Columns)
        {
            delete[] dgList->dg_Columns;
            dgList->dg_Columns = NULL;
        }
        // Delete Items - hochan Added
        for (int i = 0; i < dgList->dg_RowPreAllocNumber; i++)
        {
            for (int j = 0; j < dgList->dg_ColumnNumber; j++)
            {
                delete[] dgList->dg_Rows[i].rowText[j];
            }
            delete[] dgList->dg_Rows[i].rowText;
            delete[] dgList->dg_Rows[i].textAlign;
            delete[] dgList->dg_Rows[i].readOnly;
        }
        // Delete rows
        free(dgList->dg_Rows);
        dgList->dg_Rows = NULL;
        dgList->dg_RowNumber = 0;
        dgList->dg_RowPreAllocNumber = 0;


        // Init DataGrid columns info
        if (numNewCols < DG_MAXCOLUMN)
            dgList->dg_ColumnNumber = numNewCols;
        else
            dgList->dg_ColumnNumber = DG_MAXCOLUMN;

        dgList->dg_Columns = new DG_COLUMN[dgList->dg_ColumnNumber];
        for (int i = 0; i < dgList->dg_ColumnNumber; i++)
        {
            dgList->dg_Columns[i].columnWidth = 50;
            dgList->dg_Columns[i].textAlign = DGTA_LEFT;
            _tcscpy(dgList->dg_Columns[i].columnText, _T(" "));
        }
        SetScrollPos(m_hWnd, SB_HORZ, 0, TRUE);
        SetScrollPos(m_hWnd, SB_VERT, 0, TRUE);
        RecalcWindow(m_hWnd);

    }
    return result;
}

BOOL CDataGrid::ResetContent()
{
    BOOL result = FALSE;
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Delete columns
        if (dgList->dg_Columns)
        {
            delete[] dgList->dg_Columns;
            dgList->dg_Columns = NULL;
        }
        // Delete Items - hochan Added
        for (int i = 0; i < dgList->dg_RowNumber; i++)
        {
            for (int j = 0; j < dgList->dg_ColumnNumber; j++)
            {
                delete[] dgList->dg_Rows[i].rowText[j];
            }
            delete[] dgList->dg_Rows[i].rowText;
            delete[] dgList->dg_Rows[i].textAlign;
            delete[] dgList->dg_Rows[i].readOnly;
        }
        // Delete rows
        free(dgList->dg_Rows);
        dgList->dg_Rows = NULL;
        dgList->dg_RowNumber = 0;
        dgList->dg_RowPreAllocNumber = 0;

        SetScrollPos(m_hWnd, SB_HORZ, 0, TRUE);
        SetScrollPos(m_hWnd, SB_VERT, 0, TRUE);
        RecalcWindow(m_hWnd);

    }
    return result;
}

HWND CDataGrid::GetWindowHandle()
{
    // Return DataGrid window handle
    return m_hWnd;
}


void InitDGGlobals(HWND hParent, HWND hWnd)
{
    DG_LIST* dgList = GetDGGrid(hWnd);
    if (dgList != NULL)
    {
        // Create fonts
        HDC hParentDC = GetDC(hParent);
        LOGFONT lf;
        lf.lfHeight = -MulDiv(10, GetDeviceCaps(hParentDC, LOGPIXELSY), 72);
        lf.lfWidth = 0;
        lf.lfEscapement = 0;
        lf.lfOrientation = 0;
        lf.lfWeight = FW_NORMAL;
        lf.lfItalic = FALSE;
        lf.lfUnderline = FALSE;
        lf.lfStrikeOut = FALSE;
        lf.lfCharSet = ANSI_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        _tcscpy(lf.lfFaceName, _T("Arial"));
        dgList->dg_hColumnFont = CreateFontIndirect(&lf);
        memcpy(&dgList->dg_LFColumnFont, &lf, sizeof(LOGFONT));
        lf.lfHeight = -MulDiv(9, GetDeviceCaps(hParentDC, LOGPIXELSY), 72);
        dgList->dg_hRowFont = CreateFontIndirect(&lf);
        memcpy(&dgList->dg_LFRowFont, &lf, sizeof(LOGFONT));
        ReleaseDC(hParent, hParentDC);

        // Create background brush
        LOGBRUSH lb;
        lb.lbColor = DGBGR_COLOR;
        lb.lbStyle = BS_SOLID;
        dgList->dg_hBgBrush = CreateBrushIndirect(&lb);

        // Create cell pen
        LOGPEN lp;
        lp.lopnColor = RGB(210, 210, 210);
        lp.lopnStyle = PS_SOLID;
        lp.lopnWidth.x = 1;
        dgList->dg_hCellPen = CreatePenIndirect(&lp);
    }
}


BOOL CDataGrid::SetColumnInfo(int columnIndex, TCHAR* columnText, int columnWidth, int textAlign)
{
    BOOL result = FALSE;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Check column index
        if (columnIndex < dgList->dg_ColumnNumber)
        {
            // Set new DataGrid column info
            _tcsncpy(dgList->dg_Columns[columnIndex].columnText, columnText, 1024);
            dgList->dg_Columns[columnIndex].columnWidth = columnWidth;
            dgList->dg_Columns[columnIndex].textAlign = textAlign;
            dgList->dg_Columns[columnIndex].pressed = false;

            result = TRUE;
        }
    }

    return result;
}


BOOL CDataGrid::SetItemInfo(int rowIndex, int columnIndex, TCHAR* itemText, int textAlign, bool readOnly)
{
    BOOL result = FALSE;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        if (SetDGItemText(m_hWnd, rowIndex, columnIndex, itemText))
        {
            dgList->dg_Rows[rowIndex].textAlign[columnIndex] = textAlign;
            if (dgList->dg_EnableEdit)
                dgList->dg_Rows[rowIndex].readOnly[columnIndex] = readOnly;

            result = TRUE;
        }
    }

    return result;
}


BOOL CDataGrid::SetItemInfo(int rowIndex, int columnIndex, TCHAR* itemText, int textAlign, bool readOnly, LPARAM lParam)
{
    BOOL result = FALSE;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        if (SetDGItemText(m_hWnd, rowIndex, columnIndex, itemText))
        {
            dgList->dg_Rows[rowIndex].textAlign[columnIndex] = textAlign;
            if (dgList->dg_EnableEdit)
                dgList->dg_Rows[rowIndex].readOnly[columnIndex] = readOnly;
            dgList->dg_Rows[rowIndex].lParam = lParam;
            result = TRUE;
        }
    }

    return result;
}


BOOL SetDGItemText(HWND hWnd, int rowIndex, int columnIndex, TCHAR* buffer)
{
    BOOL result = FALSE;

    DG_LIST* dgList = GetDGGrid(hWnd);
    if (dgList != NULL)
    {
        // Check column and row index
        if ((columnIndex < dgList->dg_ColumnNumber) && (rowIndex < dgList->dg_RowNumber) && (buffer != NULL))
        {
            // Set DataGrid row info
            //  if ( _tcslen(dgList->dg_Rows[rowIndex].rowText[columnIndex]) < _tcslen(buffer) )
            //allocate bigger memory to avoid often delete and copy
            if ((_tcslen(dgList->dg_Rows[rowIndex].rowText[columnIndex]) < _tcslen(buffer))
                && (DG_MINTEXTLEN < _tcslen(buffer)))
            {
                delete dgList->dg_Rows[rowIndex].rowText[columnIndex];
                dgList->dg_Rows[rowIndex].rowText[columnIndex] = new TCHAR[_tcslen(buffer) + 1];
            }
            if (_tcslen(buffer) == 0)
                _tcscpy(dgList->dg_Rows[rowIndex].rowText[columnIndex], _T(" "));
            else
                _tcscpy(dgList->dg_Rows[rowIndex].rowText[columnIndex], buffer);

            result = TRUE;
        }
    }

    return result;
}


BOOL CDataGrid::SetItemInfo(DG_ITEMINFO* itemInfo)
{
    BOOL result = FALSE;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        if ((itemInfo != NULL) && (itemInfo->dgItem < dgList->dg_RowNumber) && (itemInfo->dgSubitem < dgList->dg_ColumnNumber))
        {
            switch (itemInfo->dgMask)
            {
            case DG_TEXTEDIT:
            {
                // Set DataGrid item text
                SetDGItemText(m_hWnd, itemInfo->dgItem, itemInfo->dgSubitem, itemInfo->dgText);
            }
            break;

            case DG_TEXTALIGN:
            {
                // Set DataGrid item text align
                dgList->dg_Rows[itemInfo->dgItem].textAlign[itemInfo->dgSubitem] = itemInfo->dgTextAlign;
            }
            break;

            case DG_TEXTHIGHLIGHT:
            {
                // Select DataGrid item
                SelectItem(itemInfo->dgItem, itemInfo->dgSubitem);
            }
            break;

            case DG_TEXTRONLY:
            {
                // Set DataGrid item edit mode
                dgList->dg_Rows[itemInfo->dgItem].readOnly[itemInfo->dgSubitem] = itemInfo->dgReadOnly;
            }
            break;

            case DG_TEXTBGCOLOR:
            {
                // Set DataGrid row background color
                dgList->dg_Rows[itemInfo->dgItem].bgColor = itemInfo->dgBgColor;
            }
            break;
            case DG_ITEMDATA:
            {
                // Set DataGrid row background color
                dgList->dg_Rows[itemInfo->dgItem].lParam = itemInfo->lParam;
            }
            break;
            }
        }
    }

    return result;
}


BOOL CDataGrid::GetItemText(int rowIndex, int columnIndex, TCHAR* buffer, int buffer_size)
{
    BOOL result = GetDGItemText(m_hWnd, rowIndex, columnIndex, buffer, buffer_size);
    return result;
}


BOOL GetDGItemText(HWND hWnd, int rowIndex, int columnIndex, TCHAR* buffer, int buffer_size)
{
    BOOL result = FALSE;

    // Clear return buffer
    _tcscpy(buffer, _T(""));

    DG_LIST* dgList = GetDGGrid(hWnd);

    // Check column and row index
    if ((columnIndex < dgList->dg_ColumnNumber) && (rowIndex < dgList->dg_RowNumber))
    {
        // Get DataGrid item text
        _tcsncpy(buffer, dgList->dg_Rows[rowIndex].rowText[columnIndex], buffer_size);
        result = TRUE;
    }

    return result;
}


LRESULT CALLBACK DataGridProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            HDC hDC = GetDC(dgList->dg_hWnd);
            RECT rectClient;
            GetClientRect(dgList->dg_hWnd, &rectClient);
            dgList->dg_hMemBitmap = CreateCompatibleBitmap(hDC, (rectClient.right - rectClient.left), (rectClient.bottom - rectClient.top));
            dgList->dg_hMemDC = CreateCompatibleDC(hDC);
            dgList->dg_hOldMemBitmap = (HBITMAP)SelectObject(dgList->dg_hMemDC, dgList->dg_hMemBitmap);
            SetFocus(dgList->dg_hWnd);
            ReleaseDC(dgList->dg_hWnd, hDC);
        }
    }
    break;

    case WM_DESTROY:
    {
        // Delete DataGrid grid
        DestroyDGGrid(hwnd);
    }
    break;

    case WM_SIZE:
    {
        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            // Delete memory device context
            if (dgList->dg_hMemDC)
            {
                SelectObject(dgList->dg_hMemDC, dgList->dg_hOldMemBitmap);
                DeleteDC(dgList->dg_hMemDC);
                dgList->dg_hMemDC = NULL;
            }
            // Delete memory bitmap
            if (dgList->dg_hMemBitmap)
            {
                DeleteObject(dgList->dg_hMemBitmap);
                dgList->dg_hMemBitmap = NULL;
            }

            HDC hDC = GetDC(dgList->dg_hWnd);
            RECT rectClient;
            GetClientRect(dgList->dg_hWnd, &rectClient);
            dgList->dg_hMemBitmap = CreateCompatibleBitmap(hDC, (rectClient.right - rectClient.left), (rectClient.bottom - rectClient.top));
            dgList->dg_hMemDC = CreateCompatibleDC(hDC);
            dgList->dg_hOldMemBitmap = (HBITMAP)SelectObject(dgList->dg_hMemDC, dgList->dg_hMemBitmap);
            ReleaseDC(dgList->dg_hWnd, hDC);
        }
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);

        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            // Draw DataGrid rows
            DrawRows(dgList->dg_hWnd);
            // Draw DataGrid columns
            DrawColumns(dgList->dg_hWnd);

            RECT clientRect;
            GetClientRect(dgList->dg_hWnd, &clientRect);
            BitBlt(ps.hdc, 0, 0, (clientRect.right - clientRect.left), (clientRect.bottom - clientRect.top), dgList->dg_hMemDC, 0, 0, SRCCOPY);
        }

        EndPaint(hwnd, &ps);
    }
    break;

    case WM_ERASEBKGND:
    {
        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            RECT clientRect;
            GetClientRect(dgList->dg_hWnd, &clientRect);
            FillRect(dgList->dg_hMemDC, &clientRect, dgList->dg_hBgBrush);
        }
    }
    break;

    case WM_HSCROLL:
    {
        switch (LOWORD(wParam))
        {
        case SB_LINERIGHT:
        case SB_PAGERIGHT:
        {
            int OldPos = GetScrollPos(hwnd, SB_HORZ);
            SetScrollPos(hwnd, SB_HORZ, OldPos + 10, TRUE);
            int NewPos = GetScrollPos(hwnd, SB_HORZ);
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
        }
        break;

        case SB_LINELEFT:
        case SB_PAGELEFT:
        {
            int OldPos = GetScrollPos(hwnd, SB_HORZ);
            SetScrollPos(hwnd, SB_HORZ, OldPos - 10, TRUE);
            int NewPos = GetScrollPos(hwnd, SB_HORZ);
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
        }
        break;

        case SB_THUMBTRACK:
        {
            SCROLLINFO si;
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            GetScrollInfo(hwnd, SB_HORZ, &si);
            int OldPos = si.nPos;
            si.nPos = si.nTrackPos;
            int NewPos = si.nPos;
            SetScrollPos(hwnd, SB_HORZ, si.nPos, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
        }
        break;
        }

        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList->dg_Edit)
        {
            int scrollX = GetScrollPos(hwnd, SB_HORZ);
            int scrollY = GetScrollPos(hwnd, SB_VERT);
            RECT columnRect, clientRect;
            GetClientRect(hwnd, &clientRect);
            GetColumnRect(hwnd, 0, &columnRect);
            int offsetLeft = scrollX + clientRect.left;
            int offsetRight = scrollX + (clientRect.right - clientRect.left);
            int offsetTop = scrollY + clientRect.top + (columnRect.bottom - columnRect.top);
            int offsetBottom = scrollY + (clientRect.bottom - clientRect.top);
            if ((dgList->dg_EditRect.top >= offsetTop) && (dgList->dg_EditRect.bottom <= offsetBottom) &&
                (dgList->dg_EditRect.right >= offsetLeft) && (dgList->dg_EditRect.left <= offsetRight))
            {
                SetWindowPos(dgList->dg_hEditWnd, HWND_NOTOPMOST, dgList->dg_EditRect.left - scrollX, dgList->dg_EditRect.top - scrollY, dgList->dg_EditRect.right - dgList->dg_EditRect.left, dgList->dg_EditRect.bottom - dgList->dg_EditRect.top, SWP_NOZORDER | SWP_SHOWWINDOW);
                SetFocus(dgList->dg_hEditWnd);
            }
            else
                ShowWindow(dgList->dg_hEditWnd, SW_HIDE);
        }
    }
    break;

    case WM_VSCROLL:
    {
        switch (LOWORD(wParam))
        {
        case SB_LINEUP:
        {
            RECT rowRect;
            GetRowRect(hwnd, 0, &rowRect);
            int OldPos = GetScrollPos(hwnd, SB_VERT);
            int diff = (rowRect.bottom - rowRect.top) - 1;
            SetScrollPos(hwnd, SB_VERT, OldPos - diff, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
        }
        break;

        case SB_PAGEUP:
        {
            RECT rowRect, clientRect;
            GetRowRect(hwnd, 0, &rowRect);
            GetClientRect(hwnd, &clientRect);
            int OldPos = GetScrollPos(hwnd, SB_VERT);
            int diff = (clientRect.bottom - clientRect.top) / (rowRect.bottom - rowRect.top - 1) - 2;
            diff *= (rowRect.bottom - rowRect.top - 1);
            SetScrollPos(hwnd, SB_VERT, OldPos - diff, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
        }
        break;

        case SB_LINEDOWN:
        {
            RECT rowRect;
            GetRowRect(hwnd, 0, &rowRect);
            int OldPos = GetScrollPos(hwnd, SB_VERT);
            int diff = (rowRect.bottom - rowRect.top) - 1;
            SetScrollPos(hwnd, SB_VERT, OldPos + diff, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
        }
        break;

        case SB_PAGEDOWN:
        {
            RECT rowRect, clientRect;
            GetRowRect(hwnd, 0, &rowRect);
            GetClientRect(hwnd, &clientRect);
            int OldPos = GetScrollPos(hwnd, SB_VERT);
            int diff = (clientRect.bottom - clientRect.top) / (rowRect.bottom - rowRect.top - 1) - 2;
            diff *= (rowRect.bottom - rowRect.top - 1);
            SetScrollPos(hwnd, SB_VERT, OldPos + diff, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
        }
        break;

        case SB_THUMBTRACK:
        {
            RECT rowRect, clientRect, columnRect;
            GetRowRect(hwnd, 0, &rowRect);
            GetColumnRect(hwnd, 0, &columnRect);
            GetClientRect(hwnd, &clientRect);
            clientRect.top = columnRect.bottom;
            SCROLLINFO si;
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            GetScrollInfo(hwnd, SB_VERT, &si);
            int OldPos = si.nPos;
            int NewPos = si.nTrackPos;
            int diff = NewPos % (rowRect.bottom - rowRect.top - 1);
            NewPos -= diff;
            si.nPos = NewPos;
            si.nTrackPos = NewPos;
            SetScrollPos(hwnd, SB_VERT, NewPos, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
        }
        break;
        }

        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList->dg_Edit)
        {
            int scrollX = GetScrollPos(hwnd, SB_HORZ);
            int scrollY = GetScrollPos(hwnd, SB_VERT);
            RECT columnRect, clientRect;
            GetClientRect(hwnd, &clientRect);
            GetColumnRect(hwnd, 0, &columnRect);
            int offsetLeft = scrollX + clientRect.left;
            int offsetRight = scrollX + (clientRect.right - clientRect.left);
            int offsetTop = scrollY + clientRect.top + (columnRect.bottom - columnRect.top);
            int offsetBottom = scrollY + (clientRect.bottom - clientRect.top);
            if ((dgList->dg_EditRect.top >= offsetTop) && (dgList->dg_EditRect.bottom <= offsetBottom) &&
                (dgList->dg_EditRect.right >= offsetLeft) && (dgList->dg_EditRect.left <= offsetRight))
            {
                SetWindowPos(dgList->dg_hEditWnd, HWND_NOTOPMOST, dgList->dg_EditRect.left - scrollX, dgList->dg_EditRect.top - scrollY, dgList->dg_EditRect.right - dgList->dg_EditRect.left, dgList->dg_EditRect.bottom - dgList->dg_EditRect.top, SWP_NOZORDER | SWP_SHOWWINDOW);
                SetFocus(dgList->dg_hEditWnd);
            }
            else
                ShowWindow(dgList->dg_hEditWnd, SW_HIDE);
        }
    }
    break;

    case WM_MOUSEWHEEL:
    {
        short int zDelta = (short)HIWORD(wParam);
        if (zDelta > 0)
        {
            // Scroll page up
            int scrollCode = SB_PAGEUP;
            short int pos = GetScrollPos(hwnd, SB_VERT);
            SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(scrollCode, pos), (LPARAM)NULL);
        }
        else
        {
            // Scroll page down
            int scrollCode = SB_PAGEDOWN;
            short int pos = GetScrollPos(hwnd, SB_VERT);
            SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(scrollCode, pos), (LPARAM)NULL);
        }
    }
    break;

    case WM_LBUTTONDOWN:
    {
        int scrollX = GetScrollPos(hwnd, SB_HORZ);
        int scrollY = GetScrollPos(hwnd, SB_VERT);
        int xPos = LOWORD(lParam) + scrollX;
        int yPos = HIWORD(lParam) + scrollY;

        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            if (dgList->dg_Edit)
            {
                TCHAR text[1024];
                SendMessage(dgList->dg_hEditWnd, WM_GETTEXT, (WPARAM)1024, (LPARAM)text);
                ShowWindow(dgList->dg_hEditWnd, SW_HIDE);
                dgList->dg_Edit = FALSE;
                SetDGItemText(hwnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn, text);

                // Send notification to the parent window
                SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_ITEMTEXTCHANGED), (LPARAM)hwnd);
            }

            // Check for resized columns
            RECT rect;
            if (dgList->dg_Resize == FALSE)
                dgList->dg_Resize = CheckColumnResize(hwnd, xPos, yPos, &dgList->dg_Column, &rect);
            // Check for clicked columns
            if (!dgList->dg_Resize)
                dgList->dg_Click = CheckColumnClick(hwnd, xPos, yPos, &dgList->dg_Column);
            if (dgList->dg_Click)
                dgList->dg_Columns[dgList->dg_Column].pressed = true;

            // Check for selected rows
            if (!dgList->dg_Click)
            {
                if (CheckRows(hwnd, xPos, yPos, &dgList->dg_Row))
                {
                    // Send notification to the parent window
                    SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_ITEMCHANGED), (LPARAM)hwnd);
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);

            SetFocus(hwnd);

            // Capture mouse
            SetCapture(hwnd);
        }
    }
    break;

    case WM_LBUTTONUP:
    {
        int scrollX = GetScrollPos(hwnd, SB_HORZ);
        int scrollY = GetScrollPos(hwnd, SB_VERT);
        int xPos = LOWORD(lParam) + scrollX;
        int yPos = HIWORD(lParam) + scrollY;

        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            for (int i = 0; i < dgList->dg_ColumnNumber; i++)
                dgList->dg_Columns[i].pressed = false;

            if (dgList->dg_Resize)
            {
                // Send notification to the parent window
                SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_COLUMNRESIZED), (LPARAM)hwnd);
            }

            RECT rect;
            dgList->dg_Resize = FALSE;
            if ((dgList->dg_Click) && (dgList->dg_EnableSort))
            {
                SortDGItems(hwnd, dgList->dg_Column);
                dgList->dg_Click = FALSE;

                // Send notification to the parent window
                SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_COLUMNCLICKED), (LPARAM)hwnd);
            }
            dgList->dg_Cursor = CheckColumnResize(hwnd, xPos, yPos, &dgList->dg_Column, &rect);
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);

            // Release mouse
            ReleaseCapture();
        }
    }
    break;

    case WM_NCLBUTTONUP:
    {
        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            for (int i = 0; i < dgList->dg_ColumnNumber; i++)
                dgList->dg_Columns[i].pressed = false;

            if (dgList->dg_Resize)
            {
                // Send notification to the parent window
                SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_COLUMNRESIZED), (LPARAM)hwnd);
            }

            dgList->dg_Resize = FALSE;
            if ((dgList->dg_Click) && (dgList->dg_EnableSort))
            {
                SortDGItems(hwnd, dgList->dg_Column);
                dgList->dg_Click = FALSE;

                // Send notification to the parent window
                SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_COLUMNCLICKED), (LPARAM)hwnd);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }
    }
    break;

    case WM_LBUTTONDBLCLK:
    {
        /*
        DG_LIST* dgList = GetDGGrid(hwnd);
        if ((dgList != NULL) && (dgList->dg_EnableEdit))
        {
            int scrollX = GetScrollPos(hwnd, SB_HORZ);
            int scrollY = GetScrollPos(hwnd, SB_VERT);
            RECT columnRect, clientRect;
            GetClientRect(hwnd, &clientRect);
            GetColumnRect(hwnd, 0, &columnRect);
            int offsetLeft = scrollX + clientRect.left;
            int offsetRight = scrollX + (clientRect.right - clientRect.left);
            int offsetTop = scrollY + clientRect.top + (columnRect.bottom - columnRect.top);
            int offsetBottom = scrollY + (clientRect.bottom - clientRect.top);
            GetCellRect(hwnd, &dgList->dg_EditRect);
            if ((dgList->dg_EditRect.top >= offsetTop) && (dgList->dg_EditRect.bottom <= offsetBottom) &&
                (dgList->dg_EditRect.right >= offsetLeft) && (dgList->dg_EditRect.left <= offsetRight))
            {
                if (!dgList->dg_Rows[dgList->dg_SelectedRow].readOnly[dgList->dg_SelectedColumn])
                {
                    if (CheckRows(hwnd, LOWORD(lParam) + scrollX, HIWORD(lParam) + scrollY, &dgList->dg_Row))
                    {
                        TCHAR text[1024];
                        GetDGItemText(hwnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn, text, 1024);
                        SetWindowPos(dgList->dg_hEditWnd, HWND_NOTOPMOST, dgList->dg_EditRect.left - scrollX, dgList->dg_EditRect.top - scrollY, dgList->dg_EditRect.right - dgList->dg_EditRect.left, dgList->dg_EditRect.bottom - dgList->dg_EditRect.top, SWP_NOZORDER | SWP_SHOWWINDOW);
                        SendMessage(dgList->dg_hEditWnd, WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)text);
                        SetFocus(dgList->dg_hEditWnd);
                        dgList->dg_Edit = TRUE;
                    }
                }
            }
        }
        */
    }
    break;

    case WM_MOUSEMOVE:
    {
        int scrollX = GetScrollPos(hwnd, SB_HORZ);
        int scrollY = GetScrollPos(hwnd, SB_VERT);
        int xPos = LOWORD(lParam) + scrollX;
        int yPos = HIWORD(lParam) + scrollY;

        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            int oldSize, newSize;
            RECT rect;
            if (wParam == MK_LBUTTON)
            {
                if ((dgList->dg_Resize) && (dgList->dg_EnableResize))
                {
                    // Get column rectangle
                    GetColumnRect(hwnd, dgList->dg_Column, &rect);

                    RECT wndRect;
                    GetWindowRect(hwnd, &wndRect);
                    POINT pt = { LOWORD(lParam), HIWORD(lParam) };

                    if ((pt.x >= 0) && (pt.x <= 2000))
                    {
                        // Calculate new column size
                        oldSize = rect.right - rect.left;
                        newSize = xPos - rect.left;
                        if (newSize < 0)
                            newSize = 2;

                        // Resize DataGrid column
                        dgList->dg_Columns[dgList->dg_Column].columnWidth = newSize;
                        RecalcWindow(hwnd);
                        RECT clientRect, columnRect;
                        GetColumnRect(hwnd, 0, &columnRect);
                        GetClientRect(hwnd, &clientRect);
                        clientRect.top += (columnRect.bottom - columnRect.top);
                        InvalidateRect(hwnd, NULL, TRUE);
                        UpdateWindow(hwnd);

                        dgList->dg_Cursor = TRUE;
                    }
                }
                else
                    dgList->dg_Cursor = FALSE;
            }
            else if (dgList->dg_EnableResize)
                dgList->dg_Cursor = CheckColumnResize(hwnd, xPos, yPos, &dgList->dg_Column, &rect);
        }
    }
    break;

    case WM_SETCURSOR:
    {
        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            // Check cursor flag
            if ((dgList->dg_Cursor) && (LOWORD(lParam) == HTCLIENT))
                SetCursor(LoadCursor(NULL, IDC_SIZEWE));
            else
                SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
    }
    break;
    case  WM_CHAR:
    {
        UINT nChar = (UINT)wParam;
        DG_LIST* dgList = GetDGGrid(hwnd);
        if ((dgList != NULL) && (dgList->dg_EnableEdit))
        {
            int scrollX = GetScrollPos(hwnd, SB_HORZ);
            int scrollY = GetScrollPos(hwnd, SB_VERT);
            RECT columnRect, clientRect;
            GetClientRect(hwnd, &clientRect);
            GetColumnRect(hwnd, 0, &columnRect);
            int offsetLeft = scrollX + clientRect.left;
            int offsetRight = scrollX + (clientRect.right - clientRect.left);
            int offsetTop = scrollY + clientRect.top + (columnRect.bottom - columnRect.top);
            int offsetBottom = scrollY + (clientRect.bottom - clientRect.top);
            GetCellRect(hwnd, &dgList->dg_EditRect);
            if ((dgList->dg_EditRect.top >= offsetTop) && (dgList->dg_EditRect.bottom <= offsetBottom) &&
                (dgList->dg_EditRect.right >= offsetLeft) && (dgList->dg_EditRect.left <= offsetRight))
            {
                if (!dgList->dg_Rows[dgList->dg_SelectedRow].readOnly[dgList->dg_SelectedColumn])
                {
                    //if ( CheckRows( hWnd, LOWORD(lParam) + scrollX, HIWORD(lParam) + scrollY, &dgList->dg_Row ) )
                    //{
                    TCHAR text[1024];
                    // dgList->dg_Rows[j].selected = true;
                    //
                    //// Mark selection
                    //dgList->dg_Selected = &dgList->dg_Rows[j];
                    //dgList->dg_SelectedRow = j;
                    //dgList->dg_SelectedColumn = i;

                    GetDGItemText(hwnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn, text, 1024);
                    SetWindowPos(dgList->dg_hEditWnd, HWND_NOTOPMOST, dgList->dg_EditRect.left - scrollX, dgList->dg_EditRect.top - scrollY, dgList->dg_EditRect.right - dgList->dg_EditRect.left, dgList->dg_EditRect.bottom - dgList->dg_EditRect.top, SWP_NOZORDER | SWP_SHOWWINDOW);
                    SendMessage(dgList->dg_hEditWnd, WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)text);
                    SetFocus(dgList->dg_hEditWnd);
                    dgList->dg_Edit = TRUE;
                    ::SendMessage(dgList->dg_hEditWnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
                    ////if((UINT)wParam != 0)
                    ////Editֻ���������ֺ�С����
                    //if((nChar>=0 && nChar <='9') || (nChar==0x08) || (nChar==0x10))
                    //{
                    //	::SendMessage(dgList->dg_hEditWnd, WM_CHAR, wParam, 0);
                    //}

                    //else
                    //{
                    //	::MessageBox(hwnd,_T("Can only accept digital characters"), _T("Error"),MB_ICONINFORMATION|MB_OK);
                    //	//return TRUE;
                    //}
                    ////}
                }
            }
        }

    }
    break;
    case WM_KEYDOWN:
    {
        DG_LIST* dgList = GetDGGrid(hwnd);
        if (dgList != NULL)
        {
            if (dgList->dg_Selected != NULL)
            {
                /* KEY_DOWN pressed */
                switch (int(wParam))
                {
                case VK_DOWN:
                case VK_RETURN:
                {
                    OnEdited(dgList->dg_hWnd);
                    // Select next item
                    SelectNextItem(hwnd, dgList->dg_Selected);
                    EnsureVisible(hwnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn);
                }
                break;
                /* KEY_UP pressed */
                case	VK_UP:
                {
                    OnEdited(dgList->dg_hWnd);
                    // Select previous item
                    SelectPreviousItem(hwnd, dgList->dg_Selected);
                    EnsureVisible(hwnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn);
                }
                break;
                /* KEY_LEFT pressed */
                case VK_LEFT:
                {
                    // Select previous subitem
                    SelectPreviousSubitem(hwnd);
                    EnsureVisible(hwnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn);
                }
                break;
                /* KEY_RIGHT pressed */
                case VK_RIGHT:
                {
                    // Select next subitem
                    SelectNextSubitem(hwnd);
                    EnsureVisible(hwnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn);
                }
                break;
                /* KEY_PAGEUP pressed */
                case 33:
                {
                    // Scroll page up
                    int scrollCode = SB_PAGEUP;
                    short int pos = GetScrollPos(hwnd, SB_VERT);
                    SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(scrollCode, pos), (LPARAM)NULL);
                }
                break;
                /* KEY_PAGEDOWN pressed */
                case 34:
                {
                    // Scroll page up
                    int scrollCode = SB_PAGEDOWN;
                    short int pos = GetScrollPos(hwnd, SB_VERT);
                    SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(scrollCode, pos), (LPARAM)NULL);
                }
                break;
                ///* KEY_CTRL pressed KEY_CTRL=17*/
                //				case 17:
                //					{
                //						// Scroll page up
                //						SelectItemRecordKey( hWnd, int(wParam));
                //


                //					}
                //					break;
                //					/* KEY_CTRL pressed KEY_SHIFT=16*/
                                //case 16:
                                //	{
                                //		// Scroll page up
                                //		SelectItemRecordKey( hWnd, int(wParam));
                                //


                                //	}
                                //	break;
                case VK_ESCAPE:
                {
                    //::SendMessage(hWnd, WM_KEYDOWN, wParam, lParam );
                    OnUnEdited(dgList->dg_hWnd);
                }
                break;
                // case VK_RETURN:
                //{
                //	::SendMessage(hWnd, WM_KEYDOWN, wParam, lParam );
                //}
                //break;
                }

                // Send notification to the parent window
                SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_ITEMCHANGED), (LPARAM)hwnd);

                InvalidateRect(hwnd, NULL, TRUE);
                UpdateWindow(hwnd);
            }
        }
    }
    break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}


void OnEdited(HWND hWnd)
{
    DG_LIST* dgList = GetDGGrid(hWnd);
    if (dgList != NULL)
    {
        if (dgList->dg_Edit)
        {
            TCHAR text[1024];
            SendMessage(dgList->dg_hEditWnd, WM_GETTEXT, (WPARAM)1024, (LPARAM)text);
            ShowWindow(dgList->dg_hEditWnd, SW_HIDE);
            dgList->dg_Edit = FALSE;
            SetDGItemText(hWnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn, text);

            // Send notification to the parent window
            SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_ITEMTEXTCHANGED), (LPARAM)hWnd);
            //NMHDR nmhGrid;
            //nmhGrid.code = DGM_ITEMTEXTCHANGED;    // Message type defined by control.
            //nmhGrid.idFrom = GetDlgCtrlID(hWnd);
            //nmhGrid.hwndFrom = hWnd;
            //SendMessage(dgList->dg_hParent,//GetParent(m_hWnd),
            //WM_NOTIFY,
            //(WPARAM)hWnd,
            //(LPARAM)&nmhGrid);
        }
    }
}

void OnUnEdited(HWND hWnd)
{
    DG_LIST* dgList = GetDGGrid(hWnd);
    if (dgList != NULL)
    {
        if (dgList->dg_Edit)
        {
            //TCHAR text[1024];
            //SendMessage( dgList->dg_hEditWnd, WM_GETTEXT, (WPARAM)1024, (LPARAM)text );
            ShowWindow(dgList->dg_hEditWnd, SW_HIDE);
            dgList->dg_Edit = FALSE;
            //SetDGItemText( m_hWnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn, text );

            // Send notification to the parent window
            // SendMessage( dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0,DGM_ITEMTEXTCHANGED), (LPARAM)m_hWnd );
        }
    }
}

void DrawColumns(HWND hWnd)
{
    int offsetX = 0, offsetY = 0;
    RECT rect;
    SIZE size;
    TEXTMETRIC tm;
    DRAWTEXTPARAMS dtp;
    dtp.cbSize = sizeof(DRAWTEXTPARAMS);
    dtp.iLeftMargin = 5;
    dtp.iRightMargin = 5;
    dtp.iTabLength = 0;

    // Find DataGrid
    DG_LIST* dgList = GetDGGrid(hWnd);

    HDC hDC = dgList->dg_hMemDC;

    int scrollX = GetScrollPos(hWnd, SB_HORZ);
    offsetX -= scrollX;

    HFONT hOldFont = (HFONT)SelectObject(hDC, dgList->dg_hColumnFont);
    COLORREF oldTxtColor = SetTextColor(hDC, dgList->dg_ColumnTextColor);
    SetBkMode(hDC, TRANSPARENT);

    for (int i = 0; i < dgList->dg_ColumnNumber; i++)
    {
        // Get column text dimensions
        GetTextExtentPoint(hDC, dgList->dg_Columns[i].columnText, _tcslen(dgList->dg_Columns[i].columnText), &size);
        GetTextMetrics(hDC, &tm);

        // Set column rectangle
        rect.left = offsetX;
        rect.top = offsetY;
        rect.right = rect.left + dgList->dg_Columns[i].columnWidth;
        rect.bottom = rect.top + size.cy + tm.tmInternalLeading;
        offsetX = rect.right;

        if (dgList->dg_EnableSort)
        {
            // Draw sorting column
            if (!dgList->dg_Columns[i].pressed)
                DrawFrameControl(hDC, &rect, DFC_BUTTON, DFCS_BUTTONPUSH);
            else
                DrawFrameControl(hDC, &rect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_FLAT);
        }
        else
        {
            // Draw non-sorting column
            rect.bottom += 2;
            DrawEdge(hDC, &rect, EDGE_ETCHED, BF_RECT | BF_MIDDLE | BF_ADJUST);
        }

        // Draw column text
        DrawTextEx(hDC, dgList->dg_Columns[i].columnText, _tcslen(dgList->dg_Columns[i].columnText), &rect, dgList->dg_Columns[i].textAlign | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS, &dtp);
    }

    SetTextColor(hDC, oldTxtColor);
    SetBkMode(hDC, OPAQUE);
    SelectObject(hDC, hOldFont);
}


void DrawRows(HWND hWnd)
{
    int offsetX = 0, offsetY = 0, offY = 0;
    RECT rect, colRect, clientRect;
    SIZE size;
    TEXTMETRIC tm;
    DRAWTEXTPARAMS dtp;
    dtp.cbSize = sizeof(DRAWTEXTPARAMS);
    dtp.iLeftMargin = 5;
    dtp.iRightMargin = 5;
    dtp.iTabLength = 0;

    // Find DataGrid
    DG_LIST* dgList = GetDGGrid(hWnd);

    HDC hDC = dgList->dg_hMemDC;

    GetClientRect(hWnd, &clientRect);

    int scrollX = GetScrollPos(hWnd, SB_HORZ);
    offsetX -= scrollX + 1;
    int scrollY = GetScrollPos(hWnd, SB_VERT);
    offY -= scrollY;

    // Get column rectangle
    GetColumnRect(hWnd, 0, &colRect);
    offsetY += (colRect.bottom - colRect.top) - 1;

    HFONT hOldFont = (HFONT)SelectObject(hDC, dgList->dg_hRowFont);
    HPEN hOldPen;
    HPEN hBgPen = CreatePen(PS_SOLID, 1, DGBGR_COLOR);
    if (dgList->dg_EnableGrid)
        hOldPen = (HPEN)SelectObject(hDC, dgList->dg_hCellPen);
    else
        hOldPen = (HPEN)SelectObject(hDC, hBgPen);
    COLORREF oldTxtColor = SetTextColor(hDC, dgList->dg_RowTextColor);
    SetBkMode(hDC, TRANSPARENT);

    // Get row clipping boundaries
    RECT rRect, cRect;
    GetRowRect(hWnd, 0, &rRect);
    GetClientRect(hWnd, &cRect);
    int rowOffsetT = scrollY / (rRect.bottom - rRect.top - 1);
    if ((scrollY % (rRect.bottom - rRect.top - 1) != 0) && (rowOffsetT > 0))
        rowOffsetT--;
    offY += rowOffsetT * (rRect.bottom - rRect.top - 1);
    int rowOffsetB = (scrollY + cRect.bottom - cRect.top) / (rRect.bottom - rRect.top - 1);
    if (rowOffsetB > dgList->dg_RowNumber)
        rowOffsetB = dgList->dg_RowNumber;

    for (int i = rowOffsetT; i < rowOffsetB; i++)
    {
        for (int j = 0; j < dgList->dg_ColumnNumber; j++)
        {
            // Get column rectangle
            GetColumnRect(hWnd, j, &colRect);

            // Get row text dimensions
            GetTextExtentPoint(hDC, dgList->dg_Rows[i].rowText[j], _tcslen(dgList->dg_Rows[i].rowText[j]), &size);
            GetTextMetrics(hDC, &tm);

            // Set row rectangle
            rect.left = offsetX;
            rect.top = offsetY + offY;
            rect.right = rect.left + (colRect.right - colRect.left + 1);
            rect.bottom = rect.top + size.cy + tm.tmInternalLeading;
            offsetX = rect.right - 1;
            // If item is full or partially visible
            if (((rect.top >= offsetY) && (rect.bottom <= clientRect.bottom)) ||
                ((rect.top < clientRect.bottom) && (rect.bottom > clientRect.bottom)) ||
                ((rect.top < offsetY) && (rect.bottom > offsetY)))
            {
                // Check if row is selected
                if (dgList->dg_Rows[i].selected == false)
                {
                    // Draw row
                    HBRUSH hBgBrush;
                    if (!dgList->dg_Rows[i].readOnly[j])
                        hBgBrush = CreateSolidBrush(dgList->dg_Rows[i].bgColor);
                    else
                        hBgBrush = CreateSolidBrush(DGRONLY_COLOR);
                    if (dgList->dg_Rows[i].mounted == true)
                        hBgBrush = CreateSolidBrush(DGMOUNTROWBGR_COLOR);
                    HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBgBrush);
                    Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

                    SelectObject(hDC, hOldBrush);
                    DeleteObject(hBgBrush);
                    // Draw row text
                    DrawTextEx(hDC, dgList->dg_Rows[i].rowText[j], _tcslen(dgList->dg_Rows[i].rowText[j]), &rect, dgList->dg_Rows[i].textAlign[j] | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS, &dtp);
                }
                else
                {
                    // Draw row
                    HBRUSH hSelectionBrush = CreateSolidBrush(DGSELROWBGR_COLOR);//(RGB(220,230,250));
                    HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hSelectionBrush);
                    Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
                    SelectObject(hDC, hOldBrush);
                    DeleteObject(hSelectionBrush);
                    // Draw row text
                    COLORREF oldTextColor = SetTextColor(hDC, DGSELROWTXT_COLOR);//RGB(130,130,130) );
                    DrawTextEx(hDC, dgList->dg_Rows[i].rowText[j], _tcslen(dgList->dg_Rows[i].rowText[j]), &rect, dgList->dg_Rows[i].textAlign[j] | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS, &dtp);
                    SetTextColor(hDC, oldTextColor);
                }

                if ((j == dgList->dg_SelectedColumn) && (i == dgList->dg_SelectedRow))
                {
                    // Draw focus
                    RECT focusRect = rect;
                    focusRect.left += 1;
                    focusRect.right -= 1;
                    focusRect.top += 1;
                    focusRect.bottom -= 1;
                    DrawFocusRect(hDC, &focusRect);
                }
            }
        }

        offsetX = -scrollX - 1;
        offY += (rect.bottom - rect.top) - 1;
    }

    DeleteObject(hBgPen);
    SetTextColor(hDC, oldTxtColor);
    SetBkMode(hDC, OPAQUE);
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldFont);
}


BOOL CheckColumnResize(HWND hWnd, int x, int y, int* col, RECT* colRect)
{
    BOOL result = FALSE;

    RECT rect;
    SIZE size;
    int offsetX = 0, offsetY = 0;
    TEXTMETRIC tm;

    DG_LIST* dgList = GetDGGrid(hWnd);

    HDC hDC = GetDC(hWnd);

    int scrollY = GetScrollPos(hWnd, SB_VERT);
    offsetY += scrollY;

    HFONT hOldFont = (HFONT)SelectObject(hDC, dgList->dg_hColumnFont);
    dgList->dg_Cursor = FALSE;
    for (int i = 0; i < dgList->dg_ColumnNumber; i++)
    {
        GetTextExtentPoint(hDC, dgList->dg_Columns[i].columnText, _tcslen(dgList->dg_Columns[i].columnText), &size);
        GetTextMetrics(hDC, &tm);

        rect.left = offsetX;
        rect.top = offsetY;
        rect.right = rect.left + dgList->dg_Columns[i].columnWidth;
        rect.bottom = rect.top + size.cy + tm.tmInternalLeading;
        offsetX = rect.right;

        // Check mouse position
        if ((abs(rect.right - x) < 3) && (rect.top <= y) && (rect.bottom >= y))
        {
            *col = i;
            *colRect = rect;
            result = TRUE;
            break;
        }
    }
    SelectObject(hDC, hOldFont);

    ReleaseDC(hWnd, hDC);

    return result;
}


BOOL CheckColumnClick(HWND hWnd, int x, int y, int* col)
{
    BOOL result = FALSE;

    POINT pt = { x, y };
    RECT rect;
    SIZE size;
    int offsetX = 0, offsetY = 0;
    TEXTMETRIC tm;

    DG_LIST* dgList = GetDGGrid(hWnd);

    HDC hDC = GetDC(hWnd);
    *col = -1;

    int scrollY = GetScrollPos(hWnd, SB_VERT);
    offsetY += scrollY;

    HFONT hOldFont = (HFONT)SelectObject(hDC, dgList->dg_hColumnFont);
    dgList->dg_Cursor = FALSE;
    for (int i = 0; i < dgList->dg_ColumnNumber; i++)
    {
        GetTextExtentPoint(hDC, dgList->dg_Columns[i].columnText, _tcslen(dgList->dg_Columns[i].columnText), &size);
        GetTextMetrics(hDC, &tm);

        rect.left = offsetX;
        rect.top = offsetY;
        rect.right = rect.left + dgList->dg_Columns[i].columnWidth;
        rect.bottom = rect.top + size.cy + tm.tmInternalLeading;
        offsetX = rect.right;

        // Check mouse position
        if (PtInRect(&rect, pt))
        {
            *col = i;
            result = TRUE;
            break;
        }
    }
    SelectObject(hDC, hOldFont);

    ReleaseDC(hWnd, hDC);

    return result;
}


void GetColumnRect(HWND hWnd, int col, RECT* colRect)
{
    SIZE size;
    int offsetX = 0, offsetY = 0;
    TEXTMETRIC tm;

    // Find DataGrid
    DG_LIST* dgList = GetDGGrid(hWnd);

    HDC hDC = GetDC(hWnd);

    HFONT hOldFont = (HFONT)SelectObject(hDC, dgList->dg_hColumnFont);
    for (int i = 0; i < dgList->dg_ColumnNumber; i++)
    {
        GetTextExtentPoint(hDC, dgList->dg_Columns[i].columnText, _tcslen(dgList->dg_Columns[i].columnText), &size);
        GetTextMetrics(hDC, &tm);

        if (i == col)
        {
            colRect->left = offsetX;
            colRect->top = offsetY;
            colRect->right = colRect->left + dgList->dg_Columns[i].columnWidth;
            colRect->bottom = colRect->top + size.cy + tm.tmInternalLeading;
            if (!dgList->dg_EnableSort)
                colRect->bottom += 2;
            break;
        }

        offsetX += dgList->dg_Columns[i].columnWidth;
    }
    SelectObject(hDC, hOldFont);

    ReleaseDC(hWnd, hDC);
}


int CDataGrid::GetResizedColumn()
{
    int result;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
        result = dgList->dg_Column;

    // Return resized column
    return result;
}


void GetRowRect(HWND hWnd, int row, RECT* rowRect)
{
    int offsetX = 0, offsetY = 0;
    RECT colRect;
    SIZE size;
    TEXTMETRIC tm;
    DRAWTEXTPARAMS dtp;
    dtp.cbSize = sizeof(DRAWTEXTPARAMS);
    dtp.iLeftMargin = 5;
    dtp.iRightMargin = 5;
    dtp.iTabLength = 0;

    DG_LIST* dgList = GetDGGrid(hWnd);

    HDC hDC = GetDC(hWnd);

    int scrollX = GetScrollPos(hWnd, SB_HORZ);
    offsetX -= scrollX;

    // Get column rectangle
    GetColumnRect(hWnd, 0, &colRect);
    offsetY += (colRect.bottom - colRect.top) - 1;

    HFONT hOldFont = (HFONT)SelectObject(hDC, dgList->dg_hRowFont);
    HPEN hOldPen = (HPEN)SelectObject(hDC, dgList->dg_hCellPen);
    SetBkMode(hDC, TRANSPARENT);

    for (int i = 0; i < dgList->dg_RowNumber; i++)
    {
        // Get row text dimensions
        GetTextExtentPoint(hDC, dgList->dg_Rows[i].rowText[0], _tcslen(dgList->dg_Rows[i].rowText[0]), &size);
        GetTextMetrics(hDC, &tm);

        if (i == row)
        {
            // Get row rectangle
            rowRect->left = offsetX;
            rowRect->top = offsetY;
            rowRect->right = rowRect->left + dgList->dg_Columns[0].columnWidth;
            rowRect->bottom = rowRect->top + size.cy + tm.tmInternalLeading;
            break;
        }

        offsetY += (rowRect->bottom - rowRect->top) - 1;
    }
    SetBkMode(hDC, OPAQUE);
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldFont);

    ReleaseDC(hWnd, hDC);
}


BOOL CheckRows(HWND hWnd, int x, int y, int* row)
{
    BOOL result = FALSE;

    DG_LIST* dgList = GetDGGrid(hWnd);

    HDC hDC = GetDC(hWnd);
    *row = -1;

    if (dgList->dg_Resize)
        return result;

    if (dgList->dg_RowNumber == 0)
        return false;

    if (dgList->dg_SelectedRow != -1)
    {
        // Clear selection
        dgList->dg_Rows[dgList->dg_SelectedRow].selected = false;
        dgList->dg_Selected = NULL;
        dgList->dg_SelectedRow = -1;
        dgList->dg_SelectedColumn = -1;
        return true;
    }

    int offsetX = 0, offsetY = 0, offY = 0;
    RECT rect, colRect;
    SIZE size;
    TEXTMETRIC tm;
    DRAWTEXTPARAMS dtp;
    dtp.cbSize = sizeof(DRAWTEXTPARAMS);
    dtp.iLeftMargin = 5;
    dtp.iRightMargin = 5;
    dtp.iTabLength = 0;

    int scrollX = GetScrollPos(hWnd, SB_HORZ);
    offsetX -= scrollX;
    int scrollY = GetScrollPos(hWnd, SB_VERT);
    offY -= scrollY;

    POINT pt = { x - scrollX, y };

    // Get column rectangle
    GetColumnRect(hWnd, 0, &colRect);
    offsetY += (colRect.bottom - colRect.top) - 1;

    HFONT hOldFont = (HFONT)SelectObject(hDC, dgList->dg_hRowFont);
    HPEN hOldPen = (HPEN)SelectObject(hDC, dgList->dg_hCellPen);
    SetBkMode(hDC, TRANSPARENT);

    bool found = false;
    DG_ROW* curr = dgList->dg_Rows;
    int rowInd = 0;
    for (int j = 0; j < dgList->dg_RowNumber; j++)
    {
        // Reset row selection flag
        dgList->dg_Rows[j].selected = false;

        for (int i = 0; i < dgList->dg_ColumnNumber; i++)
        {
            // Get row text dimensions
            GetTextExtentPoint(hDC, dgList->dg_Rows[j].rowText[i], _tcslen(dgList->dg_Rows[j].rowText[i]), &size);
            GetTextMetrics(hDC, &tm);

            // Set row rectangle
            rect.left = offsetX;
            rect.top = offsetY;
            rect.right = rect.left + dgList->dg_Columns[i].columnWidth;
            rect.bottom = rect.top + size.cy + tm.tmInternalLeading;
            offsetX = rect.right - 1;

            // Check if row is selected
            if ((PtInRect(&rect, pt)) && (found == false))
            {
                *row = rowInd;
                dgList->dg_Rows[j].selected = true;
                found = true;

                // Mark selection
                dgList->dg_Selected = &dgList->dg_Rows[j];
                dgList->dg_SelectedRow = j;
                dgList->dg_SelectedColumn = i;

                result = TRUE;
            }
        }

        offsetX = -scrollX;
        offsetY += (rect.bottom - rect.top) - 1;
        rowInd++;
    }

    SetBkMode(hDC, OPAQUE);
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldFont);

    ReleaseDC(hWnd, hDC);

    return result;
}


void RecalcWindow(HWND hWnd)
{
    int sizeX = 0;
    int sizeY = 0;

    DG_LIST* dgList = GetDGGrid(hWnd);
    if (dgList != NULL)
    {
        // Get DataGrid window rectangle
        RECT rect;
        GetClientRect(hWnd, &rect);

        for (int i = 0; i < dgList->dg_ColumnNumber; i++)
        {
            // Calculate total columns width
            sizeX += dgList->dg_Columns[i].columnWidth;
        }

        // Check horizontal scroll bar visibility
        int scrollDiff = (rect.right - rect.left) - sizeX;
        if (scrollDiff < 0)
        {

            // Show horizontal scroll bar
            SCROLLINFO si;
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            si.nPos = GetScrollPos(hWnd, SB_HORZ);
            si.nMin = 0;
            si.nMax = sizeX;
            si.nPage = si.nMax - si.nMin + 1 - abs(scrollDiff);
            si.nTrackPos = 0;
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
            ShowScrollBar(hWnd, SB_HORZ, TRUE);
        }
        else
            //ShowScrollBar( hWnd, SB_HORZ, FALSE );
        {   // Show horizontal scroll bar
            SCROLLINFO si;
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            si.nPos = GetScrollPos(hWnd, SB_HORZ);
            si.nMin = 0;
            si.nMax = sizeX - scrollDiff;
            si.nPage = si.nMax - si.nMin + 1 - abs(scrollDiff);
            si.nTrackPos = 0;
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
            ShowScrollBar(hWnd, SB_HORZ, FALSE);
        }

        if (dgList->dg_RowNumber > 0)
        {
            RECT colRect, rowRect;
            DRAWTEXTPARAMS dtp;
            dtp.cbSize = sizeof(DRAWTEXTPARAMS);
            dtp.iLeftMargin = 5;
            dtp.iRightMargin = 5;
            dtp.iTabLength = 0;
            // Get column rectangle
            GetColumnRect(hWnd, 0, &colRect);
            sizeY += (colRect.bottom - colRect.top) - 1;
            // Get row rectangle
            GetRowRect(hWnd, 0, &rowRect);
            sizeY += dgList->dg_RowNumber * (rowRect.bottom - rowRect.top - 1) + 1;

            // Check vertical scroll bar visibility
            scrollDiff = (rect.bottom - rect.top) - sizeY;
            if (scrollDiff < 0)
            {
                // Show vertical scroll bar
                SCROLLINFO si;
                si.cbSize = sizeof(SCROLLINFO);
                si.fMask = SIF_ALL;
                si.nPos = GetScrollPos(hWnd, SB_VERT);
                si.nMin = 0;
                si.nMax = sizeY;
                si.nPage = si.nMax - si.nMin + 1 - abs(scrollDiff);
                si.nTrackPos = 0;
                SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
                ShowScrollBar(hWnd, SB_VERT, TRUE);
            }
            else
                ShowScrollBar(hWnd, SB_VERT, FALSE);
        }
        else
            //ShowScrollBar( hWnd, SB_VERT, FALSE );
        {
            // Show vertical scroll bar
            SCROLLINFO si;
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            si.nPos = GetScrollPos(hWnd, SB_VERT);
            si.nMin = 0;
            si.nMax = sizeY - scrollDiff;
            si.nPage = si.nMax - si.nMin + 1 - abs(scrollDiff);
            si.nTrackPos = 0;
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
            ShowScrollBar(hWnd, SB_VERT, FALSE);
        }
    }
}


BOOL CDataGrid::InsertItem(TCHAR* itemText, int textAlign)
{
    BOOL result = FALSE;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        if (dgList->dg_RowNumber < DG_MAXROW)
        {
            dgList->dg_RowNumber++;

            if (dgList->dg_RowPreAllocNumber < dgList->dg_RowNumber)
            {
                int iSavedRowNumber = dgList->dg_RowPreAllocNumber;
                dgList->dg_RowPreAllocNumber += DG_ROWINCRESEONETIME;
                if (dgList->dg_Rows == NULL)
                    dgList->dg_Rows = (DG_ROW*)malloc(dgList->dg_RowPreAllocNumber * sizeof(DG_ROW));
                else
                    dgList->dg_Rows = (DG_ROW*)realloc(dgList->dg_Rows, dgList->dg_RowPreAllocNumber * sizeof(DG_ROW));

                for (int i = iSavedRowNumber; i < dgList->dg_RowPreAllocNumber; i++)
                {
                    dgList->dg_Rows[i].rowText = new TCHAR * [dgList->dg_ColumnNumber];
                    dgList->dg_Rows[i].textAlign = new int[dgList->dg_ColumnNumber];
                    dgList->dg_Rows[i].readOnly = new bool[dgList->dg_ColumnNumber];
                    for (int j = 0; j < dgList->dg_ColumnNumber; j++)
                    {
                        //dgList->dg_Rows[dgList->dg_RowNumber-1].rowText[j] = new TCHAR[_tcslen(itemText)+1];
                        dgList->dg_Rows[i].rowText[j] = new TCHAR[DG_MINTEXTLEN];//changed to avoid memeory allocate often #define DG_MINTEXTLEN 128 can be changed to suit your need
                        _tcscpy(dgList->dg_Rows[i].rowText[j], _T(" "));
                        dgList->dg_Rows[i].textAlign[j] = textAlign;
                        dgList->dg_Rows[i].readOnly[j] = false;
                    }
                    _tcscpy(dgList->dg_Rows[i].rowText[0], itemText);
                    dgList->dg_Rows[i].selected = false;
                    dgList->dg_Rows[i].bgColor = DGBGR_COLOR;
                    dgList->dg_Rows[i].lParam = -1;
                }
                // Send notification to the parent window
                //SendMessage( dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0,DGM_ITEMADDED), (LPARAM)m_hWnd );
            }
            //for ( int j=0; j<dgList->dg_ColumnNumber; j++ )
            //{
            //    _tcscpy( dgList->dg_Rows[dgList->dg_RowNumber-1].rowText[j], " " );
            //    dgList->dg_Rows[dgList->dg_RowNumber-1].textAlign[j] = textAlign;
            //    dgList->dg_Rows[dgList->dg_RowNumber-1].readOnly[j] = readOnly;
            //}
            _tcscpy(dgList->dg_Rows[dgList->dg_RowNumber - 1].rowText[0], itemText);
            dgList->dg_Rows[dgList->dg_RowNumber - 1].selected = false;
            dgList->dg_Rows[dgList->dg_RowNumber - 1].bgColor = DGBGR_COLOR;

            dgList->dg_Rows[dgList->dg_RowNumber - 1].lParam = -1;
            result = TRUE;
            // Send notification to the parent window
            //SendMessage( dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0,DGM_ITEMADDED), (LPARAM)m_hWnd );
            NMHDR nmhGrid;
            nmhGrid.code = DGM_ITEMADDED;    // Message type defined by control.
            nmhGrid.idFrom = GetDlgCtrlID(m_hWnd);
            nmhGrid.hwndFrom = m_hWnd;
            SendMessage(dgList->dg_hParent,//GetParent(m_hWnd),
                WM_NOTIFY,
                (WPARAM)m_hWnd,
                (LPARAM)&nmhGrid);

        }
    }

    return result;
}


BOOL CDataGrid::InsertItem(TCHAR* itemText, int textAlign, bool readOnly)
{
    BOOL result = FALSE;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        if (dgList->dg_RowNumber < DG_MAXROW)
        {
            dgList->dg_RowNumber++;
            if (dgList->dg_RowPreAllocNumber < dgList->dg_RowNumber)
            {
                int iSavedRowNumber = dgList->dg_RowPreAllocNumber;
                dgList->dg_RowPreAllocNumber += DG_ROWINCRESEONETIME;
                if (dgList->dg_Rows == NULL)
                    dgList->dg_Rows = (DG_ROW*)malloc(dgList->dg_RowPreAllocNumber * sizeof(DG_ROW));
                else
                    dgList->dg_Rows = (DG_ROW*)realloc(dgList->dg_Rows, dgList->dg_RowPreAllocNumber * sizeof(DG_ROW));
                for (int i = iSavedRowNumber; i < dgList->dg_RowPreAllocNumber; i++)
                {
                    dgList->dg_Rows[i].rowText = new TCHAR * [dgList->dg_ColumnNumber];
                    dgList->dg_Rows[i].textAlign = new int[dgList->dg_ColumnNumber];
                    dgList->dg_Rows[i].readOnly = new bool[dgList->dg_ColumnNumber];
                    for (int j = 0; j < dgList->dg_ColumnNumber; j++)
                    {
                        //Ϊ�˱��ⷴ������new������һ���԰��ڴ濪��һЩ��
                        //dgList->dg_Rows[dgList->dg_RowNumber-1].rowText[j] = new TCHAR[_tcslen(itemText)+1];
                        dgList->dg_Rows[i].rowText[j] = new TCHAR[DG_MINTEXTLEN];
                        _tcscpy(dgList->dg_Rows[i].rowText[j], _T(" "));
                        dgList->dg_Rows[i].textAlign[j] = textAlign;
                        dgList->dg_Rows[i].readOnly[j] = readOnly;
                    }
                    _tcscpy(dgList->dg_Rows[i].rowText[0], itemText);
                    dgList->dg_Rows[i].selected = false;
                    dgList->dg_Rows[i].mounted = false;
                    dgList->dg_Rows[i].bgColor = DGBGR_COLOR;
                    dgList->dg_Rows[i].lParam = -1;
                }
                // Send notification to the parent window
                //SendMessage( dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0,DGM_ITEMADDED), (LPARAM)m_hWnd );
            }
            //for ( int j=0; j<dgList->dg_ColumnNumber; j++ )
            //{
            //    _tcscpy( dgList->dg_Rows[dgList->dg_RowNumber-1].rowText[j], " " );
            //    dgList->dg_Rows[dgList->dg_RowNumber-1].textAlign[j] = textAlign;
            //    dgList->dg_Rows[dgList->dg_RowNumber-1].readOnly[j] = readOnly;
            //}
            _tcscpy(dgList->dg_Rows[dgList->dg_RowNumber - 1].rowText[0], itemText);
            dgList->dg_Rows[dgList->dg_RowNumber - 1].selected = false;
            dgList->dg_Rows[dgList->dg_RowNumber - 1].mounted = false;
            dgList->dg_Rows[dgList->dg_RowNumber - 1].bgColor = DGBGR_COLOR;
            dgList->dg_Rows[dgList->dg_RowNumber - 1].lParam = -1;
            result = TRUE;
            // Send notification to the parent window
            // SendMessage( dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0,DGM_ITEMADDED), (LPARAM)m_hWnd );
            NMHDR nmhGrid;
            nmhGrid.code = DGM_ITEMADDED;    // Message type defined by control.
            nmhGrid.idFrom = GetDlgCtrlID(m_hWnd);
            nmhGrid.hwndFrom = m_hWnd;
            SendMessage(dgList->dg_hParent,//GetParent(m_hWnd),
                WM_NOTIFY,
                (WPARAM)m_hWnd,
                (LPARAM)&nmhGrid);
        }
    }

    return result;
}


BOOL CDataGrid::RemoveItem(int row)
{
    BOOL result = FALSE;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if ((row >= 0) && (row < dgList->dg_RowNumber))
    {
        dgList->dg_RowNumber--;
        int i, j;
        int iNext;
        //dgList->dg_RowPreAllocNumber--;
        if (row <= dgList->dg_RowNumber - 1)
        {
            //DG_ROW dg_rowSaved;
            //dg_rowSaved.rowText = new TCHAR*[dgList->dg_ColumnNumber];
            //dg_rowSaved.textAlign = new int[dgList->dg_ColumnNumber];
            //dg_rowSaved.readOnly = new bool[dgList->dg_ColumnNumber];
            //for (  j=0; j<dgList->dg_ColumnNumber; j++ )
            //{
            // dg_rowSaved.rowText[j] = new TCHAR[DG_MINTEXTLEN];
            // _tcscpy( dg_rowSaved.rowText[j], dgList->dg_Rows[row].rowText[j] );
            // dg_rowSaved.textAlign[j] = dgList->dg_Rows[row].textAlign[j];
            // dg_rowSaved.readOnly[j] = dgList->dg_Rows[row].readOnly[j];
            //}
            //dg_rowSaved.selected = dgList->dg_Rows[row].selected;
            //dg_rowSaved.bgColor = dgList->dg_Rows[row].bgColor;
            //dg_rowSaved.lParam = dgList->dg_Rows[row].lParam;

            for (i = row; i < dgList->dg_RowNumber; i++)
            {
                // memcpy( &dgList->dg_Rows[i], &dgList->dg_Rows[i+1], sizeof(DG_ROW) );

                iNext = i + 1;
                for (j = 0; j < dgList->dg_ColumnNumber; j++)
                {
                    //_tcscpy( dgList->dg_Rows[i].rowText[j], dgList->dg_Rows[iNext].rowText[j] );
                    if ((_tcslen(dgList->dg_Rows[i].rowText[j]) < _tcslen(dgList->dg_Rows[iNext].rowText[j]))
                        && (DG_MINTEXTLEN < _tcslen(dgList->dg_Rows[iNext].rowText[j])))
                    {
                        delete dgList->dg_Rows[i].rowText[j];
                        dgList->dg_Rows[i].rowText[j] = new TCHAR[_tcslen(dgList->dg_Rows[iNext].rowText[j]) + 1];
                    }
                    if (_tcslen(dgList->dg_Rows[iNext].rowText[j]) == 0)
                        _tcscpy(dgList->dg_Rows[i].rowText[j], _T(" "));
                    else
                        _tcscpy(dgList->dg_Rows[i].rowText[j], dgList->dg_Rows[iNext].rowText[j]);
                    dgList->dg_Rows[i].textAlign[j] = dgList->dg_Rows[iNext].textAlign[j];
                    dgList->dg_Rows[i].readOnly[j] = dgList->dg_Rows[iNext].readOnly[j];
                }
                dgList->dg_Rows[i].selected = dgList->dg_Rows[iNext].selected;
                dgList->dg_Rows[i].mounted = dgList->dg_Rows[iNext].mounted;
                dgList->dg_Rows[i].bgColor = dgList->dg_Rows[iNext].bgColor;
                dgList->dg_Rows[i].lParam = dgList->dg_Rows[iNext].lParam;
            }
            //  iNext=dgList->dg_RowNumber;
            //  for (  j=0; j<dgList->dg_ColumnNumber; j++ )
            //  {
            //	  _tcscpy( dgList->dg_Rows[iNext].rowText[j], dg_rowSaved.rowText[j] );
            //	  delete [] dg_rowSaved.rowText[j];
            //	  dgList->dg_Rows[iNext].textAlign[j] = dg_rowSaved.textAlign[j];
            //	  dgList->dg_Rows[iNext].readOnly[j] = dg_rowSaved.readOnly[j];
            //  }
            //  dgList->dg_Rows[iNext].selected = dg_rowSaved.selected;
            //  dgList->dg_Rows[iNext].bgColor = dg_rowSaved.bgColor;
            //  dgList->dg_Rows[iNext].lParam = dg_rowSaved.lParam;
            //delete [] dg_rowSaved.rowText;
            //delete [] dg_rowSaved.textAlign;
            //delete [] dg_rowSaved.readOnly;
            //if (dgList->dg_RowNumber>0)
            //dgList->dg_Rows = (DG_ROW*)realloc(dgList->dg_Rows, dgList->dg_RowPreAllocNumber*sizeof(DG_ROW));

        }
        else
        {
            // if (dgList->dg_RowNumber>0)
            //dgList->dg_Rows = (DG_ROW*)realloc(dgList->dg_Rows, dgList->dg_RowPreAllocNumber*sizeof(DG_ROW));
            // assert(row < dgList->dg_RowNumber);
        }
        // Mark selection
        dgList->dg_Selected = NULL;
        dgList->dg_SelectedRow = -1;
        dgList->dg_SelectedColumn = -1;

        result = TRUE;
    }

    // Send notification to the parent window
    //SendMessage( dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0,DGM_ITEMREMOVED), (LPARAM)m_hWnd );

    NMHDR nmhGrid;
    nmhGrid.code = DGM_ITEMREMOVED;    // Message type defined by control.
    nmhGrid.idFrom = GetDlgCtrlID(m_hWnd);
    nmhGrid.hwndFrom = m_hWnd;
    SendMessage(dgList->dg_hParent,//GetParent(m_hWnd),
        WM_NOTIFY,
        (WPARAM)m_hWnd,
        (LPARAM)&nmhGrid);
    return result;
}

void CDataGrid::RemoveAllItems()
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);

    // Delete all items
    if (dgList->dg_Rows != NULL)
    {
        dgList->dg_Edit = FALSE;
        ShowWindow(dgList->dg_hEditWnd, SW_HIDE);

        // Delete Items - hochan Added
        for (int i = 0; i < dgList->dg_RowPreAllocNumber; i++) //use dg_RowPreAllocNumber not dg_RowNumber we have allocate more space
        {
            for (int j = 0; j < dgList->dg_ColumnNumber; j++)
            {
                delete[] dgList->dg_Rows[i].rowText[j];
            }
            delete[] dgList->dg_Rows[i].rowText;
            delete[] dgList->dg_Rows[i].textAlign;
            delete[] dgList->dg_Rows[i].readOnly;
        }
        // Delete rows
        free(dgList->dg_Rows);
        dgList->dg_Rows = NULL;
        dgList->dg_RowNumber = 0;
        dgList->dg_RowPreAllocNumber = 0;
        dgList->dg_Row = -1;
        SetScrollPos(m_hWnd, SB_HORZ, 0, TRUE);
        SetScrollPos(m_hWnd, SB_VERT, 0, TRUE);
        RecalcWindow(m_hWnd);
    }

    InvalidateRect(m_hWnd, NULL, TRUE);
    UpdateWindow(m_hWnd);
    NMHDR nmhGrid;
    nmhGrid.code = DGM_ITEMREMOVEDALL;    // Message type defined by control.
    nmhGrid.idFrom = GetDlgCtrlID(m_hWnd);
    nmhGrid.hwndFrom = m_hWnd;
    SendMessage(dgList->dg_hParent,//GetParent(m_hWnd),
        WM_NOTIFY,
        (WPARAM)m_hWnd,
        (LPARAM)&nmhGrid);
}

int CDataGrid::GetSelectedRow()
{
    int result = -1;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Return selected row
        result = dgList->dg_SelectedRow;
    }

    return result;
}


int CDataGrid::GetSelectedColumn()
{
    int result = -1;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Return selected column
        result = dgList->dg_SelectedColumn;
    }

    return result;
}

void CDataGrid::ResetSelection()
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        if (dgList->dg_RowNumber > 0 && dgList->dg_SelectedRow != -1)
        {
            dgList->dg_Rows[dgList->dg_SelectedRow].selected = false;
            dgList->dg_Selected = NULL;
            dgList->dg_SelectedRow = -1;
            dgList->dg_SelectedColumn = -1;
        }
    }
}

void CDataGrid::SetRowMount(DWORD deviceId, BOOL value)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        for (int i = 0; i < dgList->dg_RowNumber; i++)
        {
            if (deviceId == (DWORD)dgList->dg_Rows[i].lParam)
            {
                dgList->dg_Rows[i].mounted = value;
                break;
            }
        }
    }
}

void CDataGrid::Update()
{
    // Update DataGrid window
    RecalcWindow(m_hWnd);
    InvalidateRect(m_hWnd, NULL, TRUE);
    UpdateWindow(m_hWnd);
}


void CDataGrid::SetCompareFunction(DGCOMPARE CompareFunction)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set custom comparison function
        dgList->dg_CompareFunc = (DGCOMPARE)CompareFunction;
    }
}


void SortDGItems(HWND hWnd, int column)
{
    DG_LIST* dgList = GetDGGrid(hWnd);
    if ((dgList != NULL) && (dgList->dg_CompareFunc != NULL))
    {
        // Send notification to the parent window
        SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_STARTSORTING), (LPARAM)hWnd);

        // Sort DataGrid items
        Sort(hWnd, column, dgList->dg_RowNumber);

        // Send notification to the parent window
        SendMessage(dgList->dg_hParent, WM_COMMAND, MAKEWPARAM(0, DGM_ENDSORTING), (LPARAM)hWnd);

        // Set selection focus
        SetDGSelection(hWnd, dgList->dg_SelectedRow, dgList->dg_SelectedColumn);
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
    }
}


void Sort(HWND hWnd, int col, int size)
{
    int i;
    DG_ROW temp;

    DG_LIST* dgList = GetDGGrid(hWnd);

    for (i = (size + 1 / 2); i >= 0; i--)
        SiftDown(hWnd, i, size - 1, col);

    for (i = size - 1; i >= 1; i--)
    {
        memcpy(&temp, &dgList->dg_Rows[0], sizeof(DG_ROW));
        memcpy(&dgList->dg_Rows[0], &dgList->dg_Rows[i], sizeof(DG_ROW));
        memcpy(&dgList->dg_Rows[i], &temp, sizeof(DG_ROW));
        SiftDown(hWnd, 0, i - 1, col);
    }
}


void SiftDown(HWND hWnd, int root, int bottom, int col)
{
    int done, maxChild;
    DG_ROW temp;

    DG_LIST* dgList = GetDGGrid(hWnd);
    done = 0;
    while ((root * 2 < bottom) && (!done))
    {
        if (root * 2 == bottom)
            maxChild = root * 2;
        else if (root * 2 < bottom)
        {
            if (dgList->dg_CompareFunc(dgList->dg_Rows[root * 2].rowText[col], dgList->dg_Rows[root * 2 + 1].rowText[col], col) > 0)
                maxChild = root * 2;
            else
                maxChild = root * 2 + 1;

            if (dgList->dg_CompareFunc(dgList->dg_Rows[root].rowText[col], dgList->dg_Rows[maxChild].rowText[col], col) < 0)
            {
                memcpy(&temp, &dgList->dg_Rows[root], sizeof(DG_ROW));
                memcpy(&dgList->dg_Rows[root], &dgList->dg_Rows[maxChild], sizeof(DG_ROW));
                memcpy(&dgList->dg_Rows[maxChild], &temp, sizeof(DG_ROW));
                root = maxChild;
            }
            else
                done = 1;
        }
        else
            done = 1;
    }
}


void SetDGSelection(HWND hWnd, int rowIndex, int columnIndex)
{
    DG_LIST* dgList = GetDGGrid(hWnd);

    bool found = false;
    for (int j = 0; j < dgList->dg_RowNumber; j++)
    {
        if ((dgList->dg_Rows[j].selected) && (!found))
        {
            // Mark selection
            dgList->dg_Selected = &dgList->dg_Rows[j];
            dgList->dg_SelectedRow = j;
            dgList->dg_SelectedColumn = columnIndex;
            found = true;
            break;
        }

        if (found)
            break;
    }
}


void SelectNextItem(HWND hWnd, DG_ROW* item)
{
    DG_LIST* dgList = GetDGGrid(hWnd);

    for (int i = 0; i < dgList->dg_RowNumber; i++)
    {
        if (((&dgList->dg_Rows[i]) == dgList->dg_Selected) && (i < dgList->dg_RowNumber - 1))
        {
            // Set selected item
            dgList->dg_Rows[i].selected = false;
            dgList->dg_Rows[i + 1].selected = true;
            dgList->dg_Selected = &dgList->dg_Rows[i + 1];
            dgList->dg_SelectedRow = i + 1;
            break;
        }
    }
}


void SelectPreviousItem(HWND hWnd, DG_ROW* item)
{
    DG_LIST* dgList = GetDGGrid(hWnd);

    for (int i = 0; i < dgList->dg_RowNumber; i++)
    {
        if (((&dgList->dg_Rows[i]) == dgList->dg_Selected) && (i > 0))
        {
            // Set selected item
            dgList->dg_Rows[i].selected = false;
            dgList->dg_Rows[i - 1].selected = true;
            dgList->dg_Selected = &dgList->dg_Rows[i - 1];
            dgList->dg_SelectedRow = i - 1;
            break;
        }
    }
}


void SelectNextSubitem(HWND hWnd)
{
    DG_LIST* dgList = GetDGGrid(hWnd);

    // Set selected subitem
    if (dgList->dg_SelectedColumn < dgList->dg_ColumnNumber - 1)
        dgList->dg_SelectedColumn++;
    else
        dgList->dg_SelectedColumn = dgList->dg_ColumnNumber - 1;
}


void SelectPreviousSubitem(HWND hWnd)
{
    DG_LIST* dgList = GetDGGrid(hWnd);

    // Set selected subitem
    if (dgList->dg_SelectedColumn > 0)
        dgList->dg_SelectedColumn--;
    else
        dgList->dg_SelectedColumn = 0;
}


void EnsureVisible(HWND hWnd, int rowIndex, int colIndex)
{
    // Ensure DataGrid item is visible
    EnsureRowVisible(hWnd, rowIndex);
    EnsureColumnVisible(hWnd, colIndex);
}


void EnsureRowVisible(HWND hWnd, int rowIndex)
{
    // Check DataGrid row visibility
    DG_LIST* dgList = GetDGGrid(hWnd);
    RECT rowRect, clientRect, columnRect;
    GetClientRect(hWnd, &clientRect);
    GetColumnRect(hWnd, 0, &columnRect);
    int clientHeight = clientRect.bottom - clientRect.top;
    GetRowRect(hWnd, 0, &rowRect);
    int rowOffsetT = dgList->dg_SelectedRow * (rowRect.bottom - rowRect.top - 1);
    int rowOffsetB = dgList->dg_SelectedRow * (rowRect.bottom - rowRect.top - 1) + (columnRect.bottom - columnRect.top - 1);
    int scrollY = GetScrollPos(hWnd, SB_VERT);
    if (scrollY > rowOffsetT)
        SetScrollPos(hWnd, SB_VERT, rowOffsetT, TRUE);
    else if (scrollY + clientHeight < rowOffsetB + (rowRect.bottom - rowRect.top))
    {
        int pos = rowOffsetB + (rowRect.bottom - rowRect.top) - (scrollY + clientHeight);
        SetScrollPos(hWnd, SB_VERT, scrollY + pos, TRUE);
    }
}


void EnsureColumnVisible(HWND hWnd, int columnIndex)
{
    // Check DataGrid column visibility
    DG_LIST* dgList = GetDGGrid(hWnd);
    RECT columnRect, clientRect;
    GetClientRect(hWnd, &clientRect);
    int scrollX = GetScrollPos(hWnd, SB_HORZ);
    int columnOffsetL = 0;
    for (int i = 0; i < dgList->dg_SelectedColumn; i++)
    {
        GetColumnRect(hWnd, i, &columnRect);
        columnOffsetL += (columnRect.right - columnRect.left);
    }
    GetColumnRect(hWnd, dgList->dg_SelectedColumn, &columnRect);
    int columnOffsetR = columnOffsetL + (columnRect.right - columnRect.left);
    if (scrollX + clientRect.left > columnOffsetL)
    {
        int pos = scrollX + clientRect.left - columnOffsetL;
        SetScrollPos(hWnd, SB_HORZ, columnOffsetL, TRUE);
    }
    else if (scrollX + clientRect.right < columnOffsetR)
    {
        int pos = columnOffsetR - (scrollX + clientRect.right);
        SetScrollPos(hWnd, SB_HORZ, scrollX + pos, TRUE);
    }
}


void CDataGrid::SetItemBgColor(int rowIndex, COLORREF bgColor)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);

    if ((rowIndex >= 0) && (rowIndex < dgList->dg_RowNumber))
    {
        dgList->dg_Rows[rowIndex].bgColor = bgColor;
        //to be efficient do not automatic redraw, call  update() manually
      //InvalidateRect( m_hWnd, NULL, TRUE );
      //UpdateWindow(m_hWnd);
    }
}


void CDataGrid::SetItemData(int rowIndex, LPARAM lParam)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);

    if ((rowIndex >= 0) && (rowIndex < dgList->dg_RowNumber))
    {
        dgList->dg_Rows[rowIndex].lParam = lParam;
    }
}

LPARAM CDataGrid::GetItemData(int rowIndex)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if ((rowIndex >= 0) && (rowIndex < dgList->dg_RowNumber))
    {
        // Get DataGrid row text color
        return dgList->dg_Rows[rowIndex].lParam;
    }
    else
        return -1;
}
BOOL AddDGGrid(HWND hWnd, HWND hParent)
{
    BOOL result = FALSE;

    // Check number of created DataGrids
    if (g_DGGridNumber < DG_MAXGRID)
    {
        // Create new DataGrid
        DG_LIST* newDGGrid = new DG_LIST;
        newDGGrid->dg_hWnd = hWnd;
        newDGGrid->dg_hParent = hParent;
        newDGGrid->dg_Columns = NULL;
        newDGGrid->dg_Rows = NULL;
        newDGGrid->dg_ColumnNumber = 0;
        newDGGrid->dg_RowPreAllocNumber = 0;
        newDGGrid->dg_RowNumber = 0;
        newDGGrid->dg_Column = -1;
        newDGGrid->dg_Row = -1;
        newDGGrid->dg_SelectedColumn = -1;
        newDGGrid->dg_SelectedRow = -1;
        newDGGrid->dg_Selected = NULL;
        newDGGrid->dg_Cursor = FALSE;
        newDGGrid->dg_Resize = FALSE;
        newDGGrid->dg_Click = FALSE;
        newDGGrid->dg_hColumnFont = NULL;
        newDGGrid->dg_hRowFont = NULL;
        newDGGrid->dg_hBgBrush = NULL;
        newDGGrid->dg_hCellPen = NULL;
        newDGGrid->dg_hMemBitmap = NULL;
        newDGGrid->dg_hMemDC = NULL;
        newDGGrid->dg_hOldMemBitmap = NULL;
        newDGGrid->dg_Edit = FALSE;
        newDGGrid->dg_hEditWnd = NULL;
        newDGGrid->dg_EnableEdit = TRUE;
        newDGGrid->dg_EnableSort = TRUE;
        newDGGrid->dg_EnableResize = TRUE;
        newDGGrid->dg_EnableGrid = TRUE;
        newDGGrid->dg_CompareFunc = NULL;
        newDGGrid->dg_ColumnTextColor = DGTXT_COLOR;
        newDGGrid->dg_RowTextColor = DGTXT_COLOR;
        newDGGrid->next = NULL;

        DG_LIST* curr = g_DGList;
        if (curr == NULL)
            g_DGList = newDGGrid;
        else
        {
            while (curr->next != NULL)
                curr = curr->next;

            curr->next = newDGGrid;
        }

        g_DGGridNumber++;
    }

    return result;
}

//mem leak see http://www.codeguru.com/cpp/controls/controls/gridcontrol/article.php/c10319/CDataGrid-Control.htm

void DestroyDGGrid(HWND hWnd)
{
    DG_LIST* dgList = DetachDGGrid(hWnd);
    if (dgList != NULL)
    {
        // Delete columns
        if (dgList->dg_Columns)
        {
            delete[] dgList->dg_Columns;
            dgList->dg_Columns = NULL;
        }
        // Delete Items - hochan Added
        for (int i = 0; i < dgList->dg_RowPreAllocNumber; i++) //use dg_RowPreAllocNumber not dg_RowNumber we have allocate more space
        {
            for (int j = 0; j < dgList->dg_ColumnNumber; j++)
            {
                delete[] dgList->dg_Rows[i].rowText[j];
            }
            delete[] dgList->dg_Rows[i].rowText;
            delete[] dgList->dg_Rows[i].textAlign;
            delete[] dgList->dg_Rows[i].readOnly;
        }
        // Delete rows
        free(dgList->dg_Rows);
        dgList->dg_Rows = NULL;
        // Delete column font
        if (dgList->dg_hColumnFont)
        {
            DeleteObject(dgList->dg_hColumnFont);
            dgList->dg_hColumnFont = NULL;
        }
        // Delete row font
        if (dgList->dg_hRowFont)
        {
            DeleteObject(dgList->dg_hRowFont);
            dgList->dg_hRowFont = NULL;
        }
        // Delete background brush
        if (dgList->dg_hBgBrush)
        {
            DeleteObject(dgList->dg_hBgBrush);
            dgList->dg_hBgBrush = NULL;
        }
        // Delete cell pen
        if (dgList->dg_hCellPen)
        {
            DeleteObject(dgList->dg_hCellPen);
            dgList->dg_hCellPen = NULL;
        }
        // Delete memory device context
        if (dgList->dg_hMemDC)
        {
            SelectObject(dgList->dg_hMemDC, dgList->dg_hOldMemBitmap);
            DeleteDC(dgList->dg_hMemDC);
            dgList->dg_hMemDC = NULL;
        }
        // Delete memory bitmap
        if (dgList->dg_hMemBitmap)
        {
            DeleteObject(dgList->dg_hMemBitmap);
            dgList->dg_hMemBitmap = NULL;
        }

        delete dgList;
        // hochan added
        g_DGGridNumber--;
    }
}


DG_LIST* GetDGGrid(HWND hWnd)
{
    DG_LIST* result = NULL;

    if (g_DGGridNumber > 0)
    {
        // Find DataGrid
        DG_LIST* dgList = g_DGList;
        while ((dgList != NULL) && (dgList->dg_hWnd != hWnd))
            dgList = dgList->next;

        result = dgList;
    }

    return result;
}


DG_LIST* DetachDGGrid(HWND hWnd)
{
    DG_LIST* result = NULL;

    if (g_DGGridNumber > 0)
    {
        // Find DataGrid
        DG_LIST* curr, * prev;
        curr = g_DGList;
        while ((curr != NULL) && (curr->dg_hWnd != hWnd))
        {
            prev = curr;
            curr = curr->next;
        }
/*
        if (curr == g_DGList)
            g_DGList = g_DGList->next;
        else
            prev->next = curr->next;
*/

        result = curr;
    }

    return result;
}


void GetCellRect(HWND hWnd, RECT* cellRect)
{
    DG_LIST* dgList = GetDGGrid(hWnd);
    if (dgList != NULL)
    {
        RECT rowRect, columnRect;
        GetColumnRect(hWnd, dgList->dg_SelectedColumn, &columnRect);
        GetRowRect(hWnd, 0, &rowRect);
        int rowOffset = dgList->dg_SelectedRow * (rowRect.bottom - rowRect.top - 1) + (columnRect.bottom - columnRect.top - 1);
        cellRect->left = columnRect.left;
        cellRect->right = cellRect->left + (columnRect.right - columnRect.left) - 1;
        cellRect->top = rowOffset + 1;
        cellRect->bottom = cellRect->top + (rowRect.bottom - rowRect.top - 1) - 1;
    }
}


void CDataGrid::EnableSort(BOOL enable)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set sort flag
        dgList->dg_EnableSort = enable;
    }
}


void CDataGrid::EnableEdit(BOOL enable)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set edit flag
        dgList->dg_EnableEdit = enable;
    }
}


void CDataGrid::EnableResize(BOOL enable)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set resize flag
        dgList->dg_EnableResize = enable;
    }
}


void CDataGrid::EnableGrid(BOOL enable)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set grid flag
        dgList->dg_EnableGrid = enable;
    }
}


void CDataGrid::SetRowTxtColor(COLORREF txtColor)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set DataGrid Text color
        dgList->dg_RowTextColor = txtColor;
        //to be efficient do not automatic redraw, call  update() manually
        //InvalidateRect( m_hWnd, NULL, TRUE );
        //UpdateWindow(m_hWnd);
    }
}


void CDataGrid::SetColumnTxtColor(COLORREF txtColor)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set DataGrid Text color
        dgList->dg_ColumnTextColor = txtColor;
        //to be efficient do not automatic redraw, call  update() manually
       //InvalidateRect( m_hWnd, NULL, TRUE );
       //UpdateWindow(m_hWnd);
    }
}


int CDataGrid::GetRowNumber()
{
    int result;

    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Get DataGrid row number
        result = dgList->dg_RowNumber;
    }

    return result;
}


void CDataGrid::GetColumnFont(LOGFONT* lf)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Get DataGrid column font
        memcpy(lf, &dgList->dg_LFColumnFont, sizeof(LOGFONT));
    }
}


void CDataGrid::SetColumnFont(LOGFONT* lf)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set DataGrid column font
        memcpy(&dgList->dg_LFColumnFont, lf, sizeof(LOGFONT));

        if (dgList->dg_hColumnFont)
            DeleteObject(dgList->dg_hColumnFont);
        dgList->dg_hColumnFont = CreateFontIndirect(lf);
    }
}


void CDataGrid::GetRowFont(LOGFONT* lf)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Get DataGrid row font
        memcpy(lf, &dgList->dg_LFRowFont, sizeof(LOGFONT));
    }
}


void CDataGrid::SetRowFont(LOGFONT* lf)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set DataGrid row font
        memcpy(&dgList->dg_LFRowFont, lf, sizeof(LOGFONT));

        if (dgList->dg_hRowFont)
            DeleteObject(dgList->dg_hRowFont);
        dgList->dg_hRowFont = CreateFontIndirect(lf);
    }
}


COLORREF CDataGrid::GetColumnTxtColor()
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Get DataGrid column text color
        return dgList->dg_ColumnTextColor;
    }
    else
        return RGB(0, 0, 0);
}


void CDataGrid::GetItemInfo(DG_ITEMINFO* itemInfo)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Get DataGrid item info
        if ((itemInfo != NULL) && (itemInfo->dgItem < dgList->dg_RowNumber) && (itemInfo->dgSubitem < dgList->dg_ColumnNumber))
        {
            switch (itemInfo->dgMask)
            {
            case DG_TEXTEDIT:
            {
                // Get DataGrid item text
                GetDGItemText(m_hWnd, itemInfo->dgItem, itemInfo->dgSubitem, itemInfo->dgText, itemInfo->dgTextLen);
            }
            break;

            case DG_TEXTALIGN:
            {
                // Get DataGrid item text alignment
                itemInfo->dgTextAlign = dgList->dg_Rows[itemInfo->dgItem].textAlign[itemInfo->dgSubitem];
            }
            break;

            case DG_TEXTHIGHLIGHT:
            {
                // Get DataGrid row selection state
                itemInfo->dgSelected = dgList->dg_Rows[itemInfo->dgItem].selected;
            }
            break;

            case DG_TEXTRONLY:
            {
                // Get DataGrid item edit mode
                itemInfo->dgReadOnly = dgList->dg_Rows[itemInfo->dgItem].readOnly[itemInfo->dgSubitem];
            }
            break;

            case DG_TEXTBGCOLOR:
            {
                // Get DataGrid row background color
                itemInfo->dgBgColor = dgList->dg_Rows[itemInfo->dgItem].bgColor;
            }
            break;
            case DG_ITEMDATA:
            {
                // Get DataGrid row background color
                itemInfo->lParam = dgList->dg_Rows[itemInfo->dgItem].lParam;
            }
            break;

            }
        }
    }
}


COLORREF CDataGrid::GetRowTxtColor()
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Get DataGrid row text color
        return dgList->dg_RowTextColor;
    }
    else
        return RGB(0, 0, 0);
}


void CDataGrid::EnsureVisible(int row, int column)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Ensure DataGrid item is full visible
        ::EnsureVisible(m_hWnd, row, column);
    }
}


void CDataGrid::SelectItem(int row, int column)
{
    DG_LIST* dgList = GetDGGrid(m_hWnd);
    if (dgList != NULL)
    {
        // Set DataGrid selection
        for (int j = 0; j < dgList->dg_RowNumber; j++)
        {
            if (j == row)
            {
                // Mark selection
                dgList->dg_Rows[j].selected = true;
                dgList->dg_Selected = &dgList->dg_Rows[j];
                dgList->dg_SelectedRow = row;
                dgList->dg_SelectedColumn = column;
            }
            else
                dgList->dg_Rows[j].selected = false;
        }
    }
}


//Window Procedure for Edit Control
BOOL CDataGrid::SubEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WPARAM VKEY = 0;
    switch (message)
    {
    case WM_MOUSEWHEEL:

        break;
    case WM_KEYDOWN:
    {
        UINT nChar;
        switch (wParam)
        {
        case VK_RETURN:
        {
            //::SendMessage(GetParent(hWnd), WM_COMMAND, IDC_DROP, 0);
            ::SendMessage(GetParent(hWnd), WM_KEYDOWN, VK_DOWN, lParam);
            return TRUE;
        } break;
        case VK_TAB:
        {
            // SetFocus(g_hButtonDrop);
            return TRUE;
        } break;
        case VK_DOWN:
        {
            nChar = VK_DOWN;
            //::PostMessage(GetParent(hWnd),WM_KEYDOWN,nChar,0);
            ::SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, lParam);
            return TRUE;
        }
        break;
        case VK_UP:
        {
            ::SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, lParam);
            /*				nChar = VK_UP;
            ::PostMessage(GetParent(hWnd),WM_KEYDOWN,nChar,0); */
            return TRUE;
        }
        break;
        case VK_LEFT:
        {
            ::SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, lParam);
            //nChar = VK_LEFT;
            //::PostMessage(GetParent(hWnd),WM_KEYDOWN,nChar,0);
            return TRUE;
        }
        break;
        case VK_RIGHT:
        {
            //nChar = VK_RIGHT;
            //::PostMessage(GetParent(hWnd),WM_KEYDOWN,nChar,0);
            ::SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, lParam);
            return TRUE;
        }
        break;
        case VK_ESCAPE:
        {
            ::SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, lParam);
            return TRUE;
        }
        break;

        default:
            //return EditProc(hWnd, msg, wParam, lParam);
            return CallWindowProc(EditProc, hWnd, message, wParam, lParam);
        }
        return 0;
    }
    break;


    /*if(burmese.isPhoneticInput)
    {
        if(IsWindowVisible(hwndPopupMenu))
        {*/
    if (wParam == VK_DOWN)
    {

        //if(burmese.selMenu.y < (MROW -1))
        //{
        //	if(wcslen(burmese.menuItem[burmese.selMenu.x][burmese.selMenu.y+1].burmese) > 0)
        //	{
        //			  burmese.selMenu.y++;
        //			  InvalidateRgn(hwndPopupMenu,NULL,TRUE);
        //	}
        //}
        return TRUE;
    }
    else if (wParam == VK_UP)
    {

        //if(burmese.selMenu.y >0 )
        //{
        //	burmese.selMenu.y--;
        //	InvalidateRgn(hwndPopupMenu,NULL,TRUE);
        //}
        return TRUE;
    }
    else if (wParam == VK_LEFT)
    {

        //if(burmese.selMenu.x > 0 )
        //{
        //	burmese.selMenu.x--;
        //	InvalidateRgn(hwndPopupMenu,NULL,TRUE);
        //}
    }
    else if (wParam == VK_RIGHT)
    {

        //if(burmese.selMenu.x <MCOL)
        //{
        //	if(wcslen(burmese.menuItem[burmese.selMenu.x+1][burmese.selMenu.y].burmese) > 0)
        //	{
        //		  burmese.selMenu.x++;
        //		  InvalidateRgn(hwndPopupMenu,NULL,TRUE);
        //	}
        //}
    }
    return FALSE;
    //	}
    //}
    if (GetKeyState(VK_MENU) & 0x8000) VKEY |= 1; // ALT key
    if (GetKeyState(VK_CONTROL) & 0x8000) VKEY |= 2; // CTRL key
    if (GetKeyState(VK_SHIFT) & 0x8000) VKEY |= 4; // SHIFT key

    if (VKEY == 0 || VKEY == 4)
        break; //no special key, let the window handle
    case WM_CHAR:
    case WM_SYSCHAR:
        //if(burmese.isPhoneticInput)
        //{
        WORD word = LOWORD(wParam);
        UINT nChar = (UINT)wParam;

        if (nChar == VK_RETURN)
        {
            //GetParent()->SendMessage(WM_KEYDOWN,nChar,0);
            //GetParent()->SendMessage(WM_EDIT_KEEVENT,nChar,0);
            return  TRUE;
        }

        if (word == VK_RETURN || word == VK_SPACE) {//if enter or space key is pressed
            //if(IsWindowVisible(hwndPopupMenu)){

            //	SendMessage (hwndEdit, EM_REPLACESEL, 0, (LPARAM) burmese.menuItem[burmese.selMenu.x][burmese.selMenu.y].burmese);//send selected burmese TCHAR
            //	ShowWindow(hwndPopupMenu,SW_HIDE);//hide menu
            //
            //	//after sending msg, reset it
            //	burmese.charBuff[0]=NULL; //clear buffer
            //	SetWindowText(hwndStHelpPane,szHelpText);
            return FALSE;
            //}
        }

        ////Check for 0 - 9 short cut
        //if(word >= 0x30 && word <= 0x39 && burmese.charBuff[0] != 0 && !(burmese.charBuff[0] >= 0x30 && burmese.charBuff[0] <= 0x39) ){
        //		if(wcslen(burmese.menuItem[burmese.selMenu.x][burmese.selMenu.y].burmese) >0 )
        //		{
        //			burmese.selMenu.y = word - 0x30;
        //			SendMessage (hwndEdit, EM_REPLACESEL, 0, (LPARAM) burmese.menuItem[burmese.selMenu.x][burmese.selMenu.y].burmese);//send selected burmese TCHAR
        //			ShowWindow(hwndPopupMenu,SW_HIDE);//hide menu
        //
        //			//after sending msg, reset it
        //			burmese.charBuff[0]=NULL; //clear buffer
        //			SetWindowText(hwndStHelpPane,szHelpText);
        //			return FALSE;
        //		}
        //}

        if (word == VK_ESCAPE) {//if escapse key pressed

            //burmese.charBuff[0]=NULL;//clear buffer
            //ShowWindow(hwndPopupMenu,SW_HIDE);//hide menu
            //SetWindowText(hwndStHelpPane,szHelpText);
            break;

        }
        //	else if((word >='A' && word <='Z') || (word >='a' && word <='z') ||  (word >='0' && word <='9') || (word==',') || (word=='.') || (word==';') || word==VK_BACK){
        //		burmese.selMenu.x = burmese.selMenu.y= 0; //reset selection to 0 (top one), whenever new key is press
        //
        //		if(word == VK_BACK ){ //if press backspace, remove one character in buffer
        //			if(len(burmese.charBuff)>0) {
        //			  burmese.charBuff[len(burmese.charBuff)-1]=0; //if backspace remove last character in buffer
        //			}
        //		}else if(len(burmese.charBuff)<BWLEN-1){
        //			wsprintf(szTemp,L"%lC",LOWORD(wParam));
        //			wcscat(burmese.charBuff,szTemp); //append to buffer
        //		}else{
        //			//TODO: add a error message here.
        //			return FALSE;
        //		}
        //
        //		if(len(burmese.charBuff)>0){//if there is TCHAR in buffer
        //
        //			Regex re(L"[^0-9]"); /* isNumeric */
        //
        //			/* when user type only numeric */
        //			bool isDigit=!re.test(burmese.charBuff);
        //
        //			/* when user type only numeric */
        //			if(isDigit){
        //				burmese.processNumericInput();
        //
        //			/* when user type burglish text */
        //			}else if(!burmese.processPhoneticInput()){
        //				 burmese.charBuff[len(burmese.charBuff)-1]=0;
        //
        //				 //TODO: add a error message here.
        //				 return FALSE;
        //			}
        //
        //			SetWindowText(hwndStHelpPane,burmese.processPhoneticHelp());
        //			//set cursor postion
        //			GetCaretPos(&pointEditCursor);
        //			//move menu to cursor
        //			ShowWindow (hwndPopupMenu, SW_HIDE);
        //			ShowWindow (hwndPopupMenu, SW_SHOWNA);
        //
        //			return FALSE;
        //		}else{
        //
        //			  //Ignore other character like back space while popup windows active
        //			  if(IsWindowVisible(hwndPopupMenu) == TRUE)
        //			  {
        //				 ShowWindow (hwndPopupMenu, SW_HIDE);
        //				 return FALSE;
        //			  }
        //		}
        //
        //	}
        //	break;
        //}else if(burmese.isTypeWriterInput){
        //
        //	//Hide the menu
        //	ShowWindow (hwndPopupMenu, SW_HIDE);
        //
        //	//for combination keys output like Sh,ift+Q -> hta htoe + yapin
        //	if (GetKeyState(VK_SHIFT) & 0x8000) VKEY |= 4; // SHIFT key
        //
        //	burmese.processTypeWriterInput(charBurmese,LOWORD(wParam),VKEY); // prcess the input // VKEY is no need
        //
        //	if(charBurmese != NULL){
        //		wsprintf(szTemp,L"%s",charBurmese);
        //		SendMessage (hwndEdit, EM_REPLACESEL, 0, (LPARAM) szTemp);
        //
        //		return FALSE;
        //	}
        //
        //}else{
        //	ShowWindow (hwndPopupMenu, SW_HIDE);
        //	break;
        //}
    }
    return  CallWindowProc(EditProc, hWnd, message, wParam, lParam);
}
