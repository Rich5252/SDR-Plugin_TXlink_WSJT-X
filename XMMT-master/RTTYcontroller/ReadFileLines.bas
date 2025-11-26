Attribute VB_Name = "ReadFileLines"
Public Function OpenLogFileForReading() As Integer
    ' Use a built-in VB6 constant for error checking
    Const FILE_ACCESS_DENIED As Long = 70
    
    Dim FileNum As Integer
    Dim sFilePath As String
    
    ' Construct the full path to the log file
    sFilePath = App.Path & "\RTTY.ADI"
    
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


Private Sub ProcessLogFile()
    Dim iFileNum As Integer
    Dim sCurrentLine As String
    Dim bEOF As Boolean
    
    ' 1. Open the file
    iFileNum = OpenLogFileForReading()
    
    If iFileNum > 0 Then
        Debug.Print "--- Reading log.txt ---"
        
        ' 2. Loop until EOF is reached
        Do
            sCurrentLine = ReadNextLogLine(iFileNum, bEOF)
            
            If Not bEOF Then
                ' Process the line (e.g., display it, parse it)
                Debug.Print "Line: " & sCurrentLine
            End If
            
        Loop While Not bEOF ' Continue as long as EOF is NOT True
        
        ' 3. Close the file
        CloseLogFile iFileNum
        
    Else
        Debug.Print "Failed to open log file, exiting read process."
    End If
    
End Sub

