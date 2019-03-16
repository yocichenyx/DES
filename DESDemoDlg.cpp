
// DESDemoDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "DESDemo.h"
#include "DESDemoDlg.h"
#include "afxdialogex.h"
#include <bitset>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()

//数据
public:
};
//选取的文件名、密钥,1-要加解密的文件，2-存储位置文件
CString strPathName, strPathName2;

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDESDemoDlg 对话框



CDESDemoDlg::CDESDemoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DESDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDESDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_file);
	DDX_Control(pDX, IDC_EDIT2, m_file2);
}

BEGIN_MESSAGE_MAP(CDESDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON3, &CDESDemoDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &CDESDemoDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDESDemoDlg::OnBnClickedButton2)
	ON_EN_CHANGE(IDC_EDIT1, &CDESDemoDlg::OnEnChangeEdit1)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDESDemoDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CDESDemoDlg 消息处理程序

BOOL CDESDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//设置combo默认值
	SetDlgItemText(IDC_COMBO1, L"一重一密");

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDESDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDESDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDESDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//加密
void CDESDemoDlg::OnBnClickedButton3()
{
	//获取下拉菜单的值,形成调用外部*.exe参数
	CString text;
	//1-加密文件，2-存储位置，3-加密类型
	CString para = strPathName + " " + strPathName2 + " ";
	GetDlgItem(IDC_COMBO1)->GetWindowText(text);
	if(text == "一重一密")
		para += "1";
	else if (text == "两重两密")
		para += "2";
	else if (text == "三重两密")
		para += "32";
	else if (text == "三重三密")
		para += "33";
	else if (text == "五重三密")
		para += "53";
	else if (text == "五重五密")
		para += "55";
	else if (text == "七重四密")
		para += "7";
	else
		para += "1";//默认为一重一密

	HINSTANCE hNewExe = ShellExecute(this->m_hWnd, _T("open"), _T("D:\\homework\\IS\\DESRaw\\Debug\\DESRaw.exe"), para, NULL, SW_HIDE);
	//"D:\\homework\\IS\\plain.txt D:\\homework\\IS\\cipher.txt D:\\homework\\IS\\newplain.txt"
	//状态判断
	if ((DWORD)hNewExe > 32)
	{
		MessageBeep(0x00000000L);
		MessageBox(_T("DES Succeed!"), MB_OK);
	}
	else
	{
		MessageBeep(0x00000010L);
		MessageBox(_T("DES Error!"), MB_OK);
	}
}

//浏览文件
void CDESDemoDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, NULL/*默认文件类型*/, NULL/*默认文件名*/, OFN_HIDEREADONLY/*样式，这里设置为“隐藏只读”*/, _T("所有文件(*.*)|*.*||")/*文件类型列表*/, NULL, NULL, FALSE/*指定文件打开对话框是否为Vista样式*/);
	if (dlg.DoModal() == IDOK)//如果用户在文件对话框中点击了“确定”按钮
	{
		strPathName = dlg.GetPathName();//获取文件路径到strPathName
		m_file.SetWindowText(strPathName);//显示文件路径到编辑框
	}
}

//解密
void CDESDemoDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, NULL/*默认文件类型*/, NULL/*默认文件名*/, OFN_HIDEREADONLY/*样式，这里设置为“隐藏只读”*/, _T("所有文件(*.*)|*.*||")/*文件类型列表*/, NULL, NULL, FALSE/*指定文件打开对话框是否为Vista样式*/);
	if (dlg.DoModal() == IDOK)//如果用户在文件对话框中点击了“确定”按钮
	{
		strPathName2 = dlg.GetPathName();//获取文件路径到strPathName
		m_file2.SetWindowText(strPathName2);//显示文件路径到编辑框
	}
}


void CDESDemoDlg::OnEnChangeEdit1()
{
	// TODO:  在此添加控件通知处理程序代码
}


void CDESDemoDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
}
