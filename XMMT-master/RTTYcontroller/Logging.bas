Attribute VB_Name = "Logging"
Option Explicit

Const maxCallsigns = 1000
Private LoggedCallsigns(maxCallsigns - 1) As String
Private LoggedFrequencies(maxCallsigns - 1) As Double
Private nCallsigns As Integer


Public FileSys As Object                        'File Sys object


Public Sub CheckAndCreateLogFile(ByVal sFileName As String)
    Dim sFilePath As String
    Dim FileNum As Integer
    
    ' Construct the full path using App.Path
    sFilePath = App.Path & "\" & sFileName
    
    ' Use Dir function to check if the file exists.
    ' Dir returns the filename if it exists, or an empty string otherwise.
    If Dir(sFilePath) = "" Then
        
        ' --- File does NOT exist: Create it and write the header ---
        
        FileNum = FreeFile ' Get a free file number
        
        ' Open For Output: This CREATES the file and prepares it for writing from the start.
        Open sFilePath For Output As #FileNum
        
        ' Write the header lines
        Print #FileNum, "RTTY Controller"
        Print #FileNum, "ID_WINLOG32"
        Print #FileNum, "<PROGRAMID:8>WINLOG32"
        Print #FileNum, "<PROGRAMVERSION:3>7.3"
        Print #FileNum, "WINLOG32 (G0CUZ) c 1999 - 2025  version 7.3.50"
        Print #FileNum, "CALL          G4AHN"
        Print #FileNum, "<EOH>"
        Print #FileNum, ""
        
        Close #FileNum
        Debug.Print "Created new log file with header at: " & sFilePath
        
    Else
        ' File already exists, do nothing or proceed to append later.
        Debug.Print "Log file already exists."
    End If
End Sub



' Usage example:
Sub LogNewQSO(ByVal sFileName As String)
    Dim tmp As String
    Dim kHz As String

    LogData sFileName, "<CALL:" + Trim(Str$(Len(RTTYMain.txtCallsign.Text))) + ">" + UCase(RTTYMain.txtCallsign.Text) + vbTab
    LogData sFileName, "<QSO_DATE:8>" + GetFormattedDateString() + vbTab
    LogData sFileName, "<QSO_TIME:4>" + GetCurrentUtcTimeHHMM_API() + vbTab
    
    'freq stored as MHz but to 1 Hz resolution. Reduce to kHz resolution for log
    kHz = Trim(Str$((Int((Val(RTTYMain.txtFreq.Text) * 1000 + 0.5)) / 1000)))           'MHz in kHz resolution
    LogData sFileName, "<FREQ:" + Trim(Str$(Len(kHz))) + ":N>" + kHz + vbTab
    
    LogData sFileName, "<MODE:4>RTTY" + vbTab
    tmp = RTTYMain.txtReportOut.Text
    If Len(RTTYMain.txtNumberOut.Text) > 0 Then tmp = tmp + " " + RTTYMain.txtNumberOut.Text
    LogData sFileName, "<RST_SENT:" + Trim(Str$(Len(tmp))) + ">" + UCase(tmp) + vbTab
    tmp = RTTYMain.txtReportIn.Text
    If Len(RTTYMain.txtNumberIn.Text) > 0 Then tmp = tmp + " " + RTTYMain.txtNumberIn.Text
    LogData sFileName, "<RST_RCVD:" + Trim(Str$(Len(tmp))) + ">" + UCase(tmp) + vbTab
    LogData sFileName, "<TX_PWR:3>100" + vbTab
    LogData sFileName, "<COMMENT:" + Trim(Str$(Len(RTTYMain.txtLoc.Text))) + ">" + RTTYMain.txtLoc.Text + vbTab
    LogData sFileName, "<APP_WL32_REMARKS:" + Trim(Str$(Len(RTTYMain.txtRem.Text))) + ">" + RTTYMain.txtRem.Text + vbTab
    
    LogData sFileName, "<EOR>" + vbCrLf
End Sub




Public Function ReadCallsignsFromLogFile(strFileName As String) As Integer
    Dim iFileNum As Integer
    Dim sCurrentLine As String
    Dim bEOF As Boolean
    Dim sPos As Integer
    Dim ePos As Integer
    Dim nChar As Integer
    Dim tmp As String
    
    ' 1. Open the file
    iFileNum = OpenLogFileForReading(strFileName)
    nCallsigns = 0
    
    If iFileNum > 0 Then
        Debug.Print "--- Reading log.txt ---"

        
        ' 2. Loop until EOF is reached
        sCurrentLine = ReadNextLogLine(iFileNum, bEOF)
        Do While Not bEOF
            
            tmp = GetElementFromLog(sCurrentLine, "<CALL:")
            If tmp <> "" And nCallsigns < maxCallsigns Then                   'blank by mistake?
                LoggedCallsigns(nCallsigns) = tmp
                LoggedFrequencies(nCallsigns) = Val(GetElementFromLog(sCurrentLine, "<FREQ:"))
                Debug.Print "Line" & Str$(nCallsigns + 1) & ": " & LoggedCallsigns(nCallsigns) & vbTab & LoggedFrequencies(nCallsigns)
                nCallsigns = nCallsigns + 1
            End If
            
            sCurrentLine = ReadNextLogLine(iFileNum, bEOF)
        Loop
        
        ' 3. Close the file
        CloseLogFile iFileNum
        
    Else
        Debug.Print "Failed to open log file, exiting read process."
    End If
    
    ReadCallsignsFromLogFile = nCallsigns
End Function

Public Function GetElementFromLog(strLogLine As String, element As String) As String
    Dim tmp As String
    Dim sPos As Integer
    Dim ePos As Integer
    Dim nChar As Integer
    
    tmp = ""
    If strLogLine <> "" Then
        ' Process the line and extract Callsign
        sPos = InStr(1, strLogLine, element)           'eg "<CALL:")
        If sPos > 0 Then
            sPos = sPos + Len(element)
            ePos = InStr(sPos, strLogLine, ">")
            If ePos > 0 Then
                nChar = Val(Mid$(strLogLine, sPos, ePos - sPos))      'number of char in callsign
                tmp = Mid$(strLogLine, ePos + 1, nChar)
            End If
        End If
    End If
    GetElementFromLog = tmp
End Function


Public Function OpenLogFileForReading(strFileName As String) As Integer
    ' Use a built-in VB6 constant for error checking
    Const FILE_ACCESS_DENIED As Long = 70
    
    Dim FileNum As Integer
    Dim sFilePath As String
    
    ' Construct the full path to the log file
    sFilePath = App.Path & "\" & strFileName
    
    ' Get a free file number for exclusive use
    FileNum = FreeFile
    
    On Error Resume Next ' Enable error handling
    
    ' Open the file for sequential reading
    Open sFilePath For Input As #FileNum
    
    ' Check for file errors (like "File not found" or "Access denied")
    If Err.Number <> 0 Then
        If Err.Number = 53 Then ' File not found
            Debug.Print "Log file not found at: " & sFilePath
        ElseIf Err.Number = FILE_ACCESS_DENIED Then
            Debug.Print "Error: Access denied to log file."
        Else
            Debug.Print "Error opening file (" & Err.Number & "): " & Err.Description
        End If
        FileNum = 0 ' Return 0 to indicate failure
    End If
    
    On Error GoTo 0 ' Disable error handling
    
    OpenLogFileForReading = FileNum
End Function

Public Function ReadNextLogLine(ByVal FileNum As Integer, ByRef EOFReached As Boolean) As String
    
    Dim sLine As String
    
    ' Check if the file is still open and valid
    If FileNum <= 0 Then
        EOFReached = True
        ReadNextLogLine = ""
        Exit Function
    End If
    
    ' Check for the End-Of-File marker before reading
    If EOF(FileNum) Then
        EOFReached = True
        ReadNextLogLine = ""
    Else
        ' Read one line of text into the string variable
        Line Input #FileNum, sLine
        ReadNextLogLine = sLine
    End If
    
End Function

Public Sub CloseLogFile(ByVal FileNum As Integer)
    
    If FileNum > 0 Then
        ' Close the file using the specific file number
        Close #FileNum
        Debug.Print "Log file closed."
    End If
    
End Sub

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


Public Function nCallsLogged() As Integer
    nCallsLogged = nCallsigns
End Function

Public Function DupeIndex(Callsign As String, FreqHz As String) As Integer
    Dim i As Integer
    Dim LogMHz As Integer
    Dim chkMHz As Integer
    
    DupeIndex = -1       'no dupe
    For i = 0 To nCallsigns - 1
        chkMHz = Int(Val(FreqHz))
        LogMHz = Int(LoggedFrequencies(i))
        'check callsign and band
        If UCase(LoggedCallsigns(i)) = UCase(Callsign) And chkMHz = LogMHz Then
            DupeIndex = i
            Exit For
        End If
    Next i
    
End Function
