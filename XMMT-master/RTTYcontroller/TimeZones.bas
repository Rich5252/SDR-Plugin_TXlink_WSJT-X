Attribute VB_Name = "TimeZones"
Option Explicit

' --- API Declarations for Time Zone Calculation ---
Private Type SYSTEMTIME
    wYear As Integer
    wMonth As Integer
    wDayOfWeek As Integer
    wDay As Integer
    wHour As Integer
    wMinute As Integer
    wSecond As Integer
    wMilliseconds As Integer
End Type

' --- Existing Declarations (for context) ---
Private Type TIME_ZONE_INFORMATION
    Bias As Long
    StandardName(0 To 31) As Integer
    StandardDate As SYSTEMTIME ' Requires SYSTEMTIME definition
    StandardBias As Long
    DaylightName(0 To 31) As Integer
    DaylightDate As SYSTEMTIME ' Requires SYSTEMTIME definition
    DaylightBias As Long
End Type

' API Function Declaration
Private Declare Function GetTimeZoneInformation Lib "kernel32" ( _
    lpTimeZoneInformation As TIME_ZONE_INFORMATION) As Long

' Time Zone Constants
Private Const TIME_ZONE_ID_UNKNOWN As Long = 0
Private Const TIME_ZONE_ID_STANDARD As Long = 1
Private Const TIME_ZONE_ID_DAYLIGHT As Long = 2

Public Function GetFormattedDateString() As String
    
    ' Declares a string variable to hold the final formatted date
    Dim sFormattedDate As String
    
    ' The Format$ function uses custom codes:
    ' YYYY = Four-digit year
    ' MM   = Two-digit month (01-12)
    ' DD   = Two-digit day (01-31)
    sFormattedDate = Format$(Date, "YYYYMMDD")
    
    ' Return the result
    GetFormattedDateString = sFormattedDate
    
End Function

Public Function GetCurrentUtcTimeHHMM_API() As String
    
    Dim tzi As TIME_ZONE_INFORMATION
    Dim biasMinutes As Long ' Total bias in minutes
    Dim dtUTC As Date
    Dim lngRet As Long
    
    ' Call the API to populate the Time Zone Information structure
    lngRet = GetTimeZoneInformation(tzi)
    
    ' Start with the base time zone bias (difference between local time and UTC)
    biasMinutes = tzi.Bias
    
    ' Check if Daylight Saving Time is currently active
    If lngRet = TIME_ZONE_ID_DAYLIGHT Then
        ' If DST is active, add the DaylightBias (usually -60 minutes) to the total bias.
        ' tzi.DaylightBias is usually a negative value, e.g., -60 minutes, which
        ' means you subtract 60 minutes from the bias. Since VB6 handles the time
        ' arithmetic, we just add the defined bias.
        biasMinutes = biasMinutes + tzi.DaylightBias
    End If
    
    ' biasMinutes now holds the total minutes difference between local time and UTC.
    ' This bias is in the sense of: Local Time - Bias = UTC.
    
    ' Calculate UTC: Subtract the total bias (in minutes) from the current time.
    ' Since VB6's DateAdd function is in hours, convert minutes to a fraction of a day:
    ' 1 minute = 1 / (24 * 60) = 1/1440
    dtUTC = DateAdd("n", biasMinutes, Now)
    
    ' Format the result as HHMM
    GetCurrentUtcTimeHHMM_API = Format$(dtUTC, "HHMM")
    
End Function



