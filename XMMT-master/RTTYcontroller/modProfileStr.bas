Attribute VB_Name = "modProfileStr"
Option Explicit

'Get returns length of string
Public Declare Function GetPrivateProfileString Lib "kernel32" Alias "GetPrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As Any, ByVal lpDefault As String, ByVal lpReturnedString As String, ByVal nSize As Long, ByVal lpFileName As String) As Long
Public Declare Function WritePrivateProfileString Lib "kernel32" Alias "WritePrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As Any, ByVal lpString As Any, ByVal lpFileName As String) As Long

Public strPPS As String * 100

'usage
'X = GetPrivateProfileString("Font006", "Height", "", strPPS, Len(strPPS), FilePath)
'H$ = Left$(Temp, X)

Public Function GetPPS$(ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpDefault As String, ByVal lpFileName As String)
    Dim X As Long
    On Error Resume Next
    
    X = GetPrivateProfileString(lpApplicationName, lpKeyName, lpDefault, strPPS, Len(strPPS), lpFileName)
    GetPPS$ = Left$(strPPS, X)

End Function


Public Function WritePPS(ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpString As String, ByVal lpFileName As String) As Long
'just to shorten name
    WritePPS = WritePrivateProfileString(lpApplicationName, lpKeyName, lpString, lpFileName)
End Function
