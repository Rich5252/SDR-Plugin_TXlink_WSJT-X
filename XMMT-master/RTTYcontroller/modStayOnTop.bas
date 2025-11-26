Attribute VB_Name = "modStayOnTop"

Public Declare Function SetWindowPos Lib "user32" (ByVal hwnd As Long, ByVal hWndInsertAfter As Long, ByVal X As Long, ByVal y As Long, ByVal cx As Long, ByVal cy As Long, ByVal wFlags As Long) As Long


Sub AlwaysOnTop(frmID As Form)
    Const SWP_NOMOVE = 2
    Const SWP_NOSIZE = 1
    Const FLAGS = SWP_NOMOVE Or SWP_NOSIZE
    Const HWND_TOPMOST = -1
    Const HWND_NOTOPMOST = -2
    OnTop = SetWindowPos(frmID.hwnd, HWND_TOPMOST, 0, 0, 0, 0, FLAGS)
End Sub


Sub NotAlwaysOnTop(frmID As Form)
    Const SWP_NOMOVE = 2
    Const SWP_NOSIZE = 1
    Const FLAGS = SWP_NOMOVE Or SWP_NOSIZE
    Const HWND_TOPMOST = -1
    Const HWND_NOTOPMOST = -2
    OnTop = SetWindowPos(frmID.hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, FLAGS)
End Sub
