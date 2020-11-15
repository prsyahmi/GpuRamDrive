#ifndef _DATAGRID_H_
#define _DATAGRID_H_
//can be changed const
//RGB colors like DGBGR_COLOR/DGTXT_COLOR/DGRONLY_COLOR
//DG_ROWINCRESEONETIME used one time allocate multiple rows for grid, to avoid memory allocate so often
//DG_MINTEXTLEN used to allocate cell maxium text length, also avoid allocate memory so often
//
//http://www.codeproject.com/KB/miscctrl/DataGridControl.aspx
//http://www.codeguru.com/cpp/controls/controls/gridcontrol/article.php/c10319/CDataGrid-Control.htm
//changed since origin version
//fixed bug of memory leak in DestroyDGGrid/RemoveItem/RemoveAllItems()
//changed code to support UNICODE/MultiByte by using TCHAR.
//InsertItem allocate fixed size string to avoid memeory allocate too often ,#define DG_MINTEXTLEN 128 can be changed to suit your need
//to avoid often allocate memory define DG_ROWINCRESEONETIME, so one time it will allocate DG_ROWINCRESEONETIME rows in grid.
//add function SetItemData/GetItemData to support Item Data, so add variable LPARAM lParam
//add OnEdited/OnUnEdited to support edit in grid
//add help function ResetGridColumns/ResetContent to help quickly clean grid.
//add help function SetItemInfo/GetItemInfo to help get Iteminfo plus ItemData
//add help function InsertItem to insertitem with Readly option.
//GetEditSafeHwnd/GetSafeHwnd to get Handle of Grid or Edit in the grid.

#include <windows.h>
//
//#ifdef UNICODE 
//#define _tcslen     wcslen
//#define _tcscpy     wcscpy
//#define _tcscpy_s   wcscpy_s
//#define _tcsncpy    wcsncpy
//#define _tcsncpy_s  wcsncpy_s
//#define _tcscat     wcscat
//#define _tcscat_s   wcscat_s
//#define _tcsupr     wcsupr
//#define _tcsupr_s   wcsupr_s
//#define _tcslwr     wcslwr
//#define _tcslwr_s   wcslwr_s
//#define _stprintf_s swprintf_s
//#define _stprintf   swprintf
//#define _tprintf    wprintf
//#define _vstprintf_s    vswprintf_s
//#define _vstprintf      vswprintf
//#define _tscanf     wscanf
//#define TCHAR wchar_t
//#else
//#define _tcslen     strlen
//#define _tcscpy     strcpy
//#define _tcscpy_s   strcpy_s
//#define _tcsncpy    strncpy
//#define _tcsncpy_s  strncpy_s
//#define _tcscat     strcat
//#define _tcscat_s   strcat_s
//#define _tcsupr     strupr
//#define _tcsupr_s   strupr_s
//#define _tcslwr     strlwr
//#define _tcslwr_s   strlwr_s
//#define _stprintf_s sprintf_s
//#define _stprintf   sprintf
//#define _tprintf    printf
//#define _vstprintf_s    vsprintf_s
//#define _vstprintf      vsprintf
//#define _tscanf     scanf
//#define TCHAR char
//#endif

#define			WM_CANCEL_EDIT			WM_USER + 202
#define			WM_EDITED				WM_USER + 203

/* DataGrid definitions */
#define DG_MAXGRID          20
#define DG_MAXCOLUMN        1000
#define DG_MAXROW           32000
#define DG_ROWINCRESEONETIME 50  //One time allocate rows
#define DG_MINTEXTLEN       128 //one cell max text length
#define DGTA_LEFT           DT_LEFT
#define DGTA_CENTER         DT_CENTER
#define DGTA_RIGHT          DT_RIGHT
#define DGBGR_COLOR         RGB(255,255,255)
#define DGTXT_COLOR         RGB(0,0,0)
#define DGRONLY_COLOR       RGB(220,220,220)

#define DGSELROWBGR_COLOR         RGB(10,10,200) //  220,230,250
#define DGSELROWTXT_COLOR         RGB(250,200,200 ) // 130,130,130

/* DataGrid messages */
#define DGM_ITEMCHANGED          0x0001
#define DGM_ITEMTEXTCHANGED      0x0002
#define DGM_ITEMADDED            0x0003
#define DGM_ITEMREMOVED          0x0004
#define DGM_COLUMNRESIZED        0x0005
#define DGM_COLUMNCLICKED        0x0006
#define DGM_STARTSORTING         0x0007
#define DGM_ENDSORTING           0x0008
#define DGM_ITEMREMOVEDALL       0x0009


/* Custom callback procedure */
typedef int(CALLBACK* DGCOMPARE)(TCHAR* item1, TCHAR* item2, int column);


/* DataGrid column definition structure */
typedef struct _DG_COLUMN
{
    TCHAR columnText[1024];
    int columnWidth;
    int textAlign;
    bool pressed;

} DG_COLUMN;


/* DataGrid row definition structure */
typedef struct _DG_ROW
{
    TCHAR** rowText;
    int* textAlign;
    bool selected;
    bool* readOnly;
    COLORREF bgColor;
    LPARAM lParam;//SetTtemData   
} DG_ROW;


/* DataGrid enumerations */
enum DG_MASK { DG_TEXTEDIT, DG_TEXTALIGN, DG_TEXTHIGHLIGHT, DG_TEXTRONLY, DG_TEXTBGCOLOR, DG_ITEMDATA };


/* DataGrid item information structure */
typedef struct _DG_ITEMINFO
{
    DG_MASK dgMask;
    int dgItem;
    int dgSubitem;
    TCHAR* dgText;
    int dgTextLen;
    int dgTextAlign;
    bool dgSelected;
    bool dgReadOnly;
    COLORREF dgBgColor;
    LPARAM lParam;//used by SetTtemData  
} DG_ITEMINFO;


/* DataGrid definition */
typedef struct _DG_LIST
{
    HWND dg_hWnd;
    HWND dg_hParent;
    DG_COLUMN* dg_Columns;
    DG_ROW* dg_Rows;
    int dg_ColumnNumber;
    int dg_RowNumber;
    int dg_RowPreAllocNumber;//one time allocate more rows, to avoid realloc
    int dg_Column;
    int dg_Row;
    int dg_SelectedColumn;
    int dg_SelectedRow;
    DG_ROW* dg_Selected;
    BOOL dg_Cursor;
    BOOL dg_Resize;
    BOOL dg_Click;
    HFONT dg_hColumnFont;
    HFONT dg_hRowFont;
    LOGFONT dg_LFColumnFont;
    LOGFONT dg_LFRowFont;
    HBRUSH dg_hBgBrush;
    HPEN dg_hCellPen;
    HBITMAP dg_hMemBitmap;
    HDC dg_hMemDC;
    HBITMAP dg_hOldMemBitmap;
    HWND dg_hEditWnd;
    RECT dg_EditRect;
    BOOL dg_Edit;
    BOOL dg_EnableEdit;
    BOOL dg_EnableSort;
    BOOL dg_EnableResize;
    BOOL dg_EnableGrid;
    DGCOMPARE dg_CompareFunc;
    COLORREF dg_ColumnTextColor;
    COLORREF dg_RowTextColor;
    struct _DG_LIST* next;
} DG_LIST;


/* DataGrid global procedures */
LRESULT CALLBACK DataGridProc(HWND, UINT, WPARAM, LPARAM);
void DrawColumns(HWND hWnd);
void DrawRows(HWND hWnd);
BOOL CheckColumnResize(HWND hWnd, int x, int y, int* col, RECT* colRect);
BOOL CheckColumnClick(HWND hWnd, int x, int y, int* col);
BOOL CheckRows(HWND hWnd, int x, int y, int* row);
void GetColumnRect(HWND hWnd, int col, RECT* colRect);
void GetRowRect(HWND hWnd, int row, RECT* rowRect);
void RecalcWindow(HWND hWnd);
void SortDGItems(HWND hWnd, int column);
void Sort(HWND hWnd, int col, int size);
void SetDGSelection(HWND hWnd, int rowIndex, int columnIndex);
void SelectNextItem(HWND hWnd, DG_ROW* item);
void SelectPreviousItem(HWND hWnd, DG_ROW* item);
void SelectNextSubitem(HWND hWnd);
void SelectPreviousSubitem(HWND hWnd);
void EnsureRowVisible(HWND hWnd, int rowIndex);
void EnsureColumnVisible(HWND hWnd, int columnIndex);
void EnsureVisible(HWND hWnd, int rowIndex, int colIndex);
void GetCellRect(HWND hWnd, RECT* cellRect);
BOOL GetDGItemText(HWND hWnd, int rowIndex, int columnIndex, TCHAR* buffer, int buffer_size);
BOOL SetDGItemText(HWND hWnd, int rowIndex, int columnIndex, TCHAR* buffer);
void InitDGGlobals(HWND hParent, HWND hWnd);
BOOL AddDGGrid(HWND hWnd, HWND hParent);
void DestroyDGGrid(HWND hWnd);
DG_LIST* GetDGGrid(HWND hWnd);
DG_LIST* DetachDGGrid(HWND hWnd);
void SiftDown(HWND hWnd, int root, int bottom, int col);
void OnEdited(HWND hwnd);//Cancel Edit Status
void OnUnEdited(HWND hwnd);//On  Edit Status


class CDataGrid
{
private:
    /* DataGrid members */
    HWND m_hWnd;
    HWND m_hParentWnd;
    HWND m_hEditWnd;
    RECT m_DGRect;

public:
    /* Basic DataGrid methods */
    CDataGrid();
    virtual ~CDataGrid();
    BOOL Create(RECT wndRect, HWND hParent, int numCols);
    void Resize();
    void Update();
    HWND GetWindowHandle();
    HWND GetSafeHwnd() { return m_hWnd; };

    HWND GetEditSafeHwnd() { return m_hEditWnd; };
    /* General DataGrid methods */
    BOOL InsertItem(TCHAR* itemText, int textAlign);
    BOOL InsertItem(TCHAR* itemText, int textAlign, bool readOnly);//added by jjwang
    void MoveNext()
    {
        // Select next item
        DG_LIST* dgList = GetDGGrid(m_hWnd);
        SelectNextItem(m_hWnd, dgList->dg_Selected);
        EnsureVisible(dgList->dg_SelectedRow, dgList->dg_SelectedColumn);
    };
    BOOL RemoveItem(int row);
    void RemoveAllItems();
    void EnableSort(BOOL enable);
    void EnableEdit(BOOL enable);
    void EnableResize(BOOL enable);
    void EnableGrid(BOOL enable);
    void EnsureVisible(int row, int column);
    void SelectItem(int row, int column);
    int GetResizedColumn();
    int GetRowNumber();
    int GetSelectedRow();
    int GetSelectedColumn();
    void SetCompareFunction(DGCOMPARE CompareFunction);

    /* DataGrid SET attribute methods */
    BOOL SetColumnInfo(int columnIndex, TCHAR* columnText, int columnWidth, int textAlign);
    void SetColumnTxtColor(COLORREF txtColor);
    void SetColumnFont(LOGFONT* lf);
    BOOL SetItemInfo(int rowIndex, int columnIndex, TCHAR* itemText, int textAlign, bool readOnly);
    BOOL SetItemInfo(int rowIndex, int columnIndex, TCHAR* itemText, int textAlign, bool readOnly, LPARAM lParam);
    BOOL SetItemInfo(DG_ITEMINFO* itemInfo);
    void SetItemBgColor(int rowIndex, COLORREF bgColor);
    void SetRowFont(LOGFONT* lf);
    void SetRowTxtColor(COLORREF txtColor);
    void SetItemData(int rowIndex, LPARAM lParam);

    /* DataGrid GET attribute methods */
    COLORREF GetColumnTxtColor();
    void GetColumnFont(LOGFONT* lf);
    void GetItemInfo(DG_ITEMINFO* itemInfo);
    BOOL GetItemText(int rowIndex, int columnIndex, TCHAR* buffer, int buffer_size);
    void GetRowFont(LOGFONT* lf);
    COLORREF GetRowTxtColor();
    LPARAM   GetItemData(int rowIndex);//GetItemInfo can get value 
    BOOL     ResetGridColumns(int numNewCols);
    BOOL     ResetContent();
    BOOL     SubEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);//Edit Ctl
};
#endif //_DATAGRID_H_