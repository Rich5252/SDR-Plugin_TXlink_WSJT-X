Attribute VB_Name = "MessagePipe"
' The name of the pipe (MUST match the name used by your C++ Server)
Public Const PIPE_NAME As String = "\\.\pipe\TXLinkCommandPipe"

'--- Constants
Public Const GENERIC_READ = &H80000000
Public Const GENERIC_WRITE = &H40000000
Public Const OPEN_EXISTING = 3
Public Const PIPE_READMODE_MESSAGE = &H2 ' Must match server's pipe mode
Public Const INVALID_HANDLE_VALUE = -1

' Constants for WaitNamedPipe
Public Const NMPWAIT_WAIT_FOREVER = &HFFFFFFFF
Public Const NMPWAIT_USE_DEFAULT_WAIT = &H0
Public Const NMPWAIT_NOWAIT = &H1

Public PipeHandle As Long

'--- Type for overlapped I/O (not used here, but good practice)
Public Type SECURITY_ATTRIBUTES
    nLength As Long
    lpSecurityDescriptor As Long
    bInheritHandle As Long
End Type

'--- API Function Declarations
Public Declare Function CreateFile Lib "kernel32" Alias "CreateFileA" ( _
    ByVal lpFileName As String, _
    ByVal dwDesiredAccess As Long, _
    ByVal dwShareMode As Long, _
    lpSecurityAttributes As Any, _
    ByVal dwCreationDisposition As Long, _
    ByVal dwFlagsAndAttributes As Long, _
    ByVal hTemplateFile As Long) As Long

Public Declare Function ReadFile Lib "kernel32" ( _
    ByVal hFile As Long, _
    lpBuffer As Any, _
    ByVal nNumberOfBytesToRead As Long, _
    lpNumberOfBytesRead As Long, _
    ByVal lpOverlapped As Long) As Long

Public Declare Function WriteFile Lib "kernel32" ( _
    ByVal hFile As Long, _
    lpBuffer As Any, _
    ByVal nNumberOfBytesToWrite As Long, _
    lpNumberOfBytesWritten As Long, _
    ByVal lpOverlapped As Long) As Long
    
Public Declare Function PeekNamedPipe Lib "kernel32" ( _
    ByVal hNamedPipe As Long, _
    lpBuffer As Any, _
    ByVal nBufferSize As Long, _
    lpBytesRead As Long, _
    lpTotalBytesAvail As Long, _
    lpBytesLeftThisMessage As Long) As Long

' Used to wait for the pipe to be available with a timeout
Public Declare Function WaitNamedPipe Lib "kernel32" Alias "WaitNamedPipeA" ( _
    ByVal lpNamedPipeName As String, _
    ByVal nTimeOut As Long) As Long

Public Declare Function CloseHandle Lib "kernel32" ( _
    ByVal hObject As Long) As Long

    


