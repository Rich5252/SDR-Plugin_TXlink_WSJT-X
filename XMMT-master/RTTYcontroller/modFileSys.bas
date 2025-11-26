Attribute VB_Name = "modFileSys"
Option Explicit

Public FileSys As Object                        'File Sys object

Sub StartFileSys()
'call this from main form_load

    Set FileSys = CreateObject("Scripting.FileSystemObject")
    
    frmStart.ComDiag.Filter = "All Files (*.*)|*.*|Text files (*.txt)|*.txt|External pulse files (*.ext)|*.ext|"
    frmStart.ComDiag.InitDir = ".\" 'GetSettingMP(App.Title, "Settings", "ExtFileRoot", "C:\VcpNT\Daten\")
    frmStart.ComDiag.FilterIndex = 1
    frmStart.ComDiag.CancelError = True

End Sub

Sub EndFileSys()
'call this from main form_unload

    'SaveSettingMP App.Title, "Settings", "ExtFileRoot", frmStart.ComDiag.InitDir
End Sub


Function OverWriteCheck(fName) As Boolean
'return True if OK to save file
    On Error GoTo errHandler
    
    OverWriteCheck = False
    If FileSys.FileExists(fName) Then       'check overwrite file
        If MsgBox("Overwrite" & " " & fName, vbOKCancel + _
                    vbDefaultButton2, "Confirm file overwrite") <> vbOK Then
            Exit Function
        End If
    End If
    OverWriteCheck = True

errHandler:
End Function


Function FileOpen() As String

    Dim sFile As String
    Dim refFile, Line, strErr, Version, Typ, i
    Const ForReading = 1, numFlag = True, strFlag = False
    On Error GoTo errHandler

    With frmStart.ComDiag
        .DialogTitle = "Open"
        .CancelError = False
        .ShowOpen
        If Len(.FileName) = 0 Then
            Exit Function
        End If
        FileOpen = .FileName
        .InitDir = FileSys.getfile(.FileName).parentfolder
    End With
    
'    Set refFile = FileSys.OpenTextFile(sFile, ForReading)
'    Do While Not refFile.AtEndOfStream
'        Line = refFile.ReadLine
'    Loop
    
errHandler:
End Function


Sub FileSaveAs()
    Dim i, refFile, fName
    
    On Error GoTo errHandler
    frmStart.ComDiag.FileName = ""
    frmStart.ComDiag.ShowSave
    fName = frmStart.ComDiag.FileName
    If FileSys.FileExists(fName) Then
        If MsgBox("Overwrite?", vbOKCancel + vbDefaultButton2) <> vbOK Then
            Exit Sub    'check overwrite file
        End If
    End If

    Set refFile = FileSys.CreateTextFile(fName, True) 'overwrites existing
    FileSys.writeline "data"
    frmStart.ComDiag.InitDir = FileSys.getfile(frmStart.ComDiag.FileName).parentfolder

errHandler:
    'cancel selected by user
End Sub

Function FileOverwrite(Fpath) As Boolean
'return true if new file or overwrite OK

    Dim i, refFile, fName
    
    On Error GoTo errHandler
    If FileSys.FileExists(Fpath) Then
        If MsgBox("Overwrite file " & Fpath & vbNewLine & "Are you sure?", vbOKCancel + vbDefaultButton2) <> vbOK Then
            Exit Function 'check overwrite file
        End If
    End If
    FileOverwrite = True
    
errHandler:
    'cancel selected by user
End Function


Function GetAppPath() As String
    Dim sAppDirectory As String
    Dim sLogFilePath As String
    
    ' 1. Get the application's directory
    sAppDirectory = App.Path
    'Debug.Print "Application Path: " & sAppDirectory
    
    ' 2. Construct a full path for a log file
    ' NOTE: Must manually add a trailing backslash if App.Path doesn't provide one (which it usually doesn't).
    If Right$(sAppDirectory, 1) <> "\" Then
        sAppDirectory = sAppDirectory & "\"
    End If
    
    sLogFilePath = sAppDirectory & "application.log"
    'Debug.Print "Log File Path: " & sLogFilePath
    
    GetAppPath = sAppDirectory
End Function


Sub LogData(ByVal LogFile As String, ByVal logMessage As String)
    Dim FileNum As Integer
    
    ' Get a free file number
    FileNum = FreeFile
    
    ' Open the file. If it doesn't exist, it's created.
    Open App.Path & "\" & LogFile For Append As #FileNum
    
    ' Write the log entry, no crlf
    Print #FileNum, logMessage;
    
    ' Always close the file handle
    Close #FileNum
End Sub

' Usage example:
Sub TestLog()
    LogData "LogFile.txt", "Application started at " & Now
    LogData "LogFile.txt", "User action successful."
End Sub

