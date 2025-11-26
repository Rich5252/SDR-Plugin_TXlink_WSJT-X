VERSION 5.00
Object = "{9FAE7EB9-098B-4952-A234-CF460419D752}#1.0#0"; "XMMT.ocx"
Begin VB.Form RTTYMain 
   Caption         =   "RTTY Controller"
   ClientHeight    =   8520
   ClientLeft      =   225
   ClientTop       =   870
   ClientWidth     =   8640
   Icon            =   "Form1.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   8520
   ScaleWidth      =   8640
   StartUpPosition =   3  'Windows Default
   Begin VB.CheckBox chkAutoIncr 
      Height          =   240
      Left            =   8070
      TabIndex        =   56
      Top             =   5235
      Value           =   1  'Checked
      Width           =   210
   End
   Begin VB.TextBox txtLoc 
      Height          =   315
      Left            =   6150
      TabIndex        =   55
      Top             =   6135
      Width           =   1305
   End
   Begin VB.TextBox txtName 
      Height          =   315
      Left            =   6150
      TabIndex        =   54
      Top             =   6525
      Width           =   1305
   End
   Begin VB.TextBox txtQTH 
      Height          =   315
      Left            =   6150
      TabIndex        =   53
      Top             =   6930
      Width           =   1305
   End
   Begin VB.TextBox txtNumberIn 
      Height          =   315
      Left            =   6840
      TabIndex        =   52
      Top             =   5730
      Width           =   615
   End
   Begin VB.TextBox txtCallsign 
      Height          =   315
      Left            =   6150
      TabIndex        =   51
      Top             =   4860
      Width           =   1305
   End
   Begin VB.TextBox txtNumberOut 
      Height          =   315
      Left            =   6840
      TabIndex        =   50
      Top             =   5310
      Width           =   615
   End
   Begin VB.TextBox txtRem 
      Height          =   345
      Left            =   165
      TabIndex        =   49
      Top             =   5310
      Width           =   4620
   End
   Begin VB.CheckBox chkEdit 
      Caption         =   "Edit Button"
      Height          =   330
      Left            =   390
      TabIndex        =   48
      Top             =   6750
      Width           =   1440
   End
   Begin VB.TextBox txtLoggedCalls 
      Alignment       =   1  'Right Justify
      Height          =   315
      Left            =   1500
      TabIndex        =   46
      Text            =   "0"
      Top             =   4860
      Width           =   675
   End
   Begin VB.CommandButton btnLogQSO 
      Caption         =   "Log QSO"
      Height          =   330
      Left            =   165
      TabIndex        =   45
      Top             =   4860
      Width           =   885
   End
   Begin VB.Timer TimerFreq 
      Interval        =   500
      Left            =   7995
      Top             =   1170
   End
   Begin VB.TextBox txtFreq 
      Height          =   315
      Left            =   3855
      TabIndex        =   43
      Top             =   4860
      Width           =   930
   End
   Begin VB.Timer Timer1 
      Interval        =   250
      Left            =   8010
      Top             =   570
   End
   Begin VB.TextBox txtReportOut 
      Height          =   315
      Left            =   6150
      TabIndex        =   39
      Text            =   "599"
      Top             =   5310
      Width           =   615
   End
   Begin VB.TextBox txtReportIn 
      Height          =   315
      Left            =   6150
      TabIndex        =   27
      Text            =   "599"
      Top             =   5730
      Width           =   615
   End
   Begin VB.CommandButton cmdClosePipe 
      Caption         =   "Disconnect"
      Height          =   375
      Left            =   7065
      TabIndex        =   26
      Top             =   8040
      Width           =   1335
   End
   Begin VB.TextBox txtData 
      Height          =   330
      Left            =   165
      TabIndex        =   25
      Text            =   "TESTING G4AHN"
      Top             =   7530
      Width           =   8220
   End
   Begin VB.CommandButton cmdSend 
      Caption         =   "cmdSend"
      Height          =   360
      Left            =   165
      TabIndex        =   24
      Top             =   7935
      Width           =   1335
   End
   Begin VB.CommandButton cmdConnect 
      Caption         =   "Connect To Pipe"
      Height          =   375
      Left            =   5385
      TabIndex        =   23
      Top             =   8040
      Width           =   1575
   End
   Begin XMMTLib.XMMXY XYscope 
      Height          =   1095
      Left            =   6585
      TabIndex        =   22
      Top             =   120
      Width           =   1245
      _Version        =   65538
      _ExtentX        =   2196
      _ExtentY        =   1931
      _StockProps     =   32
   End
   Begin XMMTLib.XMMBtn BtnM4 
      Height          =   375
      Left            =   2100
      TabIndex        =   0
      Top             =   1305
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "Bye"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnM3 
      Height          =   375
      Left            =   1620
      TabIndex        =   1
      Top             =   1305
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "Info"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnM2 
      Height          =   375
      Left            =   1140
      TabIndex        =   2
      Top             =   1305
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "Rprt"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnM1 
      Height          =   375
      Left            =   660
      TabIndex        =   3
      Top             =   1305
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "CQ"
      FaceColor       =   -2147483633
   End
   Begin VB.TextBox Text 
      BeginProperty Font 
         Name            =   "Courier New"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2955
      Left            =   105
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   4
      Top             =   1785
      Width           =   8295
   End
   Begin XMMTLib.XMMBtn BtnAFC 
      Height          =   375
      Left            =   1920
      TabIndex        =   5
      Top             =   840
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Caption         =   "AFC"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnNET 
      Height          =   375
      Left            =   1440
      TabIndex        =   6
      Top             =   840
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Caption         =   "NET"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnATC 
      Height          =   375
      Left            =   960
      TabIndex        =   12
      Top             =   840
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Caption         =   "ATC"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnBPF 
      Height          =   375
      Left            =   1920
      TabIndex        =   11
      Top             =   480
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Caption         =   "BPF"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnLMS 
      Height          =   375
      Left            =   1440
      TabIndex        =   10
      Top             =   480
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Caption         =   "LMS"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnSQ 
      Height          =   375
      Left            =   960
      TabIndex        =   9
      Top             =   480
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Caption         =   "SQ"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnHAM 
      Height          =   375
      Left            =   1920
      TabIndex        =   8
      Top             =   120
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "HAM"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnRev 
      Height          =   375
      Left            =   1440
      TabIndex        =   7
      Top             =   120
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Caption         =   "Rev"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnType 
      Height          =   375
      Left            =   960
      TabIndex        =   13
      Top             =   120
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "Typ"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnUOS 
      Height          =   375
      Left            =   120
      TabIndex        =   14
      Top             =   840
      Width           =   735
      _Version        =   65538
      _ExtentX        =   1296
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Caption         =   "UOS"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMLvl Level 
      Height          =   1095
      Left            =   2520
      TabIndex        =   16
      Top             =   120
      Width           =   255
      _Version        =   65538
      _ExtentX        =   450
      _ExtentY        =   1931
      _StockProps     =   32
      BorderStyle     =   1
   End
   Begin XMMTLib.XMMWater WaterFall 
      Height          =   375
      Left            =   2760
      TabIndex        =   17
      Top             =   840
      Width           =   3825
      _Version        =   65538
      _ExtentX        =   6747
      _ExtentY        =   661
      _StockProps     =   32
      BorderStyle     =   1
      BaseFreq        =   1800
      WidthFreq       =   800
   End
   Begin XMMTLib.XMMSpec Spectram 
      Height          =   735
      Left            =   2760
      TabIndex        =   18
      Top             =   120
      Width           =   3825
      _Version        =   65538
      _ExtentX        =   6747
      _ExtentY        =   1296
      _StockProps     =   36
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      BorderStyle     =   1
      MarkFreq        =   915
      SpaceFreq       =   1085
      BaseFreq        =   600
      WidthFreq       =   800
   End
   Begin XMMTLib.XMMBtn BtnTxOff 
      Height          =   375
      Left            =   120
      TabIndex        =   19
      Top             =   480
      Width           =   735
      _Version        =   65538
      _ExtentX        =   1296
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "TXOFF"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnTx 
      Height          =   375
      Left            =   120
      TabIndex        =   20
      Top             =   120
      Width           =   735
      _Version        =   65538
      _ExtentX        =   1296
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Caption         =   "TX"
      TextColor       =   255
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMR XMMR 
      Height          =   255
      Index           =   1
      Left            =   8160
      TabIndex        =   21
      Top             =   90
      Visible         =   0   'False
      Width           =   240
      _Version        =   65538
      _ExtentX        =   423
      _ExtentY        =   450
      _StockProps     =   0
      bInitialClose   =   0   'False
      bNotifyComChange=   0   'False
      InvokeCommand   =   "C:\Ham\MMTTY\MMTTY.EXE -m"
      Title           =   "XMMR/MMTTY engine"
   End
   Begin XMMTLib.XMMBtn btnAns 
      Height          =   375
      Left            =   135
      TabIndex        =   33
      Top             =   1305
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "Ans"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn btnAgain 
      Height          =   375
      Left            =   3270
      TabIndex        =   34
      Top             =   6120
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "AGN"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn btnTU 
      Height          =   375
      Left            =   2550
      TabIndex        =   35
      Top             =   6120
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "TU"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn btnReport 
      Height          =   375
      Left            =   1830
      TabIndex        =   36
      Top             =   6120
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "Rprt"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn btnCall 
      Height          =   375
      Left            =   1110
      TabIndex        =   37
      Top             =   6120
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "Call"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn btnCQ 
      Height          =   375
      Left            =   390
      TabIndex        =   38
      Top             =   6120
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   661
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "CQ"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn btnDecr 
      Height          =   315
      Left            =   7485
      TabIndex        =   41
      Top             =   5490
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   556
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.26
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "Dec"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn btnIncr 
      Height          =   285
      Left            =   7485
      TabIndex        =   42
      Top             =   5220
      Width           =   495
      _Version        =   65538
      _ExtentX        =   873
      _ExtentY        =   503
      _StockProps     =   4
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Toggled         =   0   'False
      Caption         =   "Inc"
      FaceColor       =   -2147483633
   End
   Begin VB.Label lblDupe 
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   7530
      TabIndex        =   47
      Top             =   4920
      Width           =   705
   End
   Begin VB.Label lblFreq 
      Alignment       =   1  'Right Justify
      Caption         =   "Freq MHz "
      Height          =   285
      Left            =   2505
      TabIndex        =   44
      Top             =   4860
      Width           =   1245
   End
   Begin VB.Label lblReportOut 
      Alignment       =   1  'Right Justify
      Caption         =   "Report Out      "
      Height          =   285
      Left            =   4575
      TabIndex        =   40
      Top             =   5325
      Width           =   1440
   End
   Begin VB.Label lblQTH 
      Alignment       =   1  'Right Justify
      Caption         =   "QTH (ALT)"
      Height          =   285
      Left            =   4545
      TabIndex        =   32
      Top             =   6930
      Width           =   1485
   End
   Begin VB.Label lblName 
      Alignment       =   1  'Right Justify
      Caption         =   "Name (SHFT/CTRL)"
      Height          =   285
      Left            =   4545
      TabIndex        =   31
      Top             =   6510
      Width           =   1485
   End
   Begin VB.Label lblLocator 
      Alignment       =   1  'Right Justify
      Caption         =   "Locator (CTRL)"
      Height          =   285
      Left            =   4800
      TabIndex        =   30
      Top             =   6135
      Width           =   1245
   End
   Begin VB.Label lblReportIn 
      Alignment       =   1  'Right Justify
      Caption         =   "Report In (SHFT)"
      Height          =   285
      Left            =   4770
      TabIndex        =   29
      Top             =   5745
      Width           =   1245
   End
   Begin VB.Label lblCallsign 
      Alignment       =   1  'Right Justify
      Caption         =   "Callsign"
      Height          =   285
      Left            =   4800
      TabIndex        =   28
      Top             =   4860
      Width           =   1245
   End
   Begin VB.Label LStatus 
      Caption         =   "Status"
      Height          =   255
      Left            =   5280
      TabIndex        =   15
      Top             =   1365
      Width           =   2460
   End
   Begin VB.Menu KV 
      Caption         =   "&View"
      Begin VB.Menu KVQ 
         Caption         =   "&High XY quality"
      End
   End
   Begin VB.Menu KU 
      Caption         =   "&User"
      Begin VB.Menu KUS 
         Caption         =   "User &1"
         Index           =   0
      End
      Begin VB.Menu KUS 
         Caption         =   "User &2"
         Index           =   1
      End
   End
   Begin VB.Menu KO 
      Caption         =   "&Option"
      Begin VB.Menu KOS 
         Caption         =   "&Setup engine..."
      End
      Begin VB.Menu KO_ 
         Caption         =   "-"
      End
      Begin VB.Menu KOI 
         Caption         =   "&Invoking command"
      End
      Begin VB.Menu KOR 
         Caption         =   "&Reconnect"
      End
   End
   Begin VB.Menu KP 
      Caption         =   "&Profiles"
      Begin VB.Menu KPS 
         Caption         =   "#1"
         Index           =   0
      End
      Begin VB.Menu KPS 
         Caption         =   "#2"
         Index           =   1
      End
      Begin VB.Menu KPS 
         Caption         =   "#3"
         Index           =   2
      End
      Begin VB.Menu KPS 
         Caption         =   "#4"
         Index           =   3
      End
      Begin VB.Menu KPS 
         Caption         =   "#5"
         Index           =   4
      End
      Begin VB.Menu KPS 
         Caption         =   "#6"
         Index           =   5
      End
      Begin VB.Menu KPS 
         Caption         =   "#7"
         Index           =   6
      End
      Begin VB.Menu KPS 
         Caption         =   "#8"
         Index           =   7
      End
      Begin VB.Menu KP_ 
         Caption         =   "-"
      End
      Begin VB.Menu KPR 
         Caption         =   "&Load file..."
      End
      Begin VB.Menu KPW 
         Caption         =   "&Write file..."
      End
   End
End
Attribute VB_Name = "RTTYMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" (Destination As Any, Source As Any, ByVal Length As Long)
Private Declare Function SendMessage Lib "user32" Alias "SendMessageA" (ByVal hwnd As Long, ByVal wMsg As Long, ByVal wParam As Long, lParam As Any) As Long

Const swAFC = &H4         '// b2 AFC
Const swNET = &H8         '// b3 NET
Const swATC = &H10        '// b4 ATC
Const swBPF = &H20        '// b5 BPF
Const swLMS = &H40        '// b6 LMS/Notch
Const swSQ = &H80         '// b7 SQ
Const swREV = &H100       '// b8 Rev
Const swUOS = &H200       '// b9 UOS
Const swNOT = &H2000      '// b13  Notch/LMS
Const swTWO = &H4000      '// b14

Const PROFILEEND = 8 - 1

Private m_nmmr(63) As Long
Private m_CurrentSwitch As Long
Private m_CurrentSwitchReady As Long

' Global variable to hold the pipe handle
Private hPipe As Long

Const LogFileName = "RTTY.ADI"
Private SettingsFileName As String


Private Function MAKELONG(ByVal low As Long, ByVal high As Long) As Long
    MAKELONG = low + high * 65536
End Function

Private Sub BtnAFC_OnLButtonClick()
    Call PostMmttySwitch(m_CurrentSwitch Xor swAFC)
End Sub

Private Sub btnAgain_OnLButtonClick()
        Call SendBtnMsg("btnAgain", "AGN AGN?")
End Sub

Private Sub btnAns_OnLButtonClick()
    Call SendBtnMsg("btnAns", "%DXcall DE %myCall %myCall K")
End Sub

Private Sub BtnATC_OnLButtonClick()
    Call PostMmttySwitch(m_CurrentSwitch Xor swATC)
End Sub

Private Sub BtnBPF_OnLButtonClick()
    Call PostMmttySwitch(m_CurrentSwitch Xor swBPF)
End Sub

Private Sub btnCall_OnLButtonClick()
    Call SendBtnMsg("btnCall", "%myCall %myCall %myCall")
End Sub

Private Sub btnCQ_OnLButtonClick()
    Call SendBtnMsg("btnCQ", "CQ TEST %myCall %myCall")
End Sub

Private Sub BtnHAM_OnLButtonClick()
    Call XMMR.Item(1).PostMmttyMessage(RXM_SETHAM, 0)
End Sub

Private Sub btnIncr_OnLButtonClick()
    On Error Resume Next
    If Val(txtNumberOut.Text) > 0 And chkAutoIncr.Value = vbChecked Then              'is it a serial number?
        txtNumberOut.Text = txtNumberOut.Text + 1

        If Len(txtNumberOut.Text) = 1 Then txtNumberOut.Text = "0" + txtNumberOut.Text
        If Len(txtNumberOut.Text) = 2 Then txtNumberOut.Text = "0" + txtNumberOut.Text
    End If
    
    txtCallsign.Text = ""
    txtNumberIn.Text = ""
End Sub

Private Sub btnDecr_OnLButtonClick()
    On Error Resume Next
    If txtNumberOut.Text = "" Then
        txtNumberOut = "001"
    Else
        If txtNumberOut.Text > 0 Then txtNumberOut.Text = txtNumberOut.Text - 1
    End If
    If Len(txtNumberOut.Text) = 1 Then txtNumberOut.Text = "0" + txtNumberOut.Text
    If Len(txtNumberOut.Text) = 2 Then txtNumberOut.Text = "0" + txtNumberOut.Text
End Sub

Private Sub BtnLMS_OnLButtonClick()
    Call PostMmttySwitch(m_CurrentSwitch Xor swLMS)
End Sub

Private Sub BtnLMS_OnRButtonClick()
    Call PostMmttySwitch(m_CurrentSwitch Xor swNOT)
End Sub

Private Sub btnLogQSO_Click()
    If txtCallsign <> "" Then
        Call CheckAndCreateLogFile(LogFileName)     'create and add new header if needed
        Call LogNewQSO(LogFileName)
        
        'clear entry for next qso
        txtCallsign.Text = ""
        txtNumberIn.Text = ""
        Call btnIncr_OnLButtonClick         'serial number update
    End If
    
    'recount entries
    txtLoggedCalls.Text = Str$(ReadCallsignsFromLogFile(LogFileName))
End Sub

Private Sub BtnM1_OnLButtonClick()
    Call SendBtnMsg("btnM1", "CQ CQ %myCall %myCall K")
End Sub

Private Sub BtnM2_OnLButtonClick()
    Call SendBtnMsg("btnM2", "%DXcall DE %myCall TNX CALL UR %RSTout %RSTout HOW COPY K")
End Sub

Private Sub BtnM3_OnLButtonClick()
    Call SendBtnMsg("btnM3", "QSL, HERE NAME %myName, QTH %myQTH, LOC %myLOC")
End Sub

Private Sub BtnM4_OnLButtonClick()
    Call SendBtnMsg("btnM4", " QSL, MNY TNX CUL 73, QRZ? DE %myCall K")
End Sub

Private Sub BtnNET_OnLButtonClick()
    Call PostMmttySwitch(m_CurrentSwitch Xor swNET)
End Sub

Private Sub btnReport_OnLButtonClick()
    Call SendBtnMsg("BtnReport", "%DXcall %RSTout %NumOut %NumOut %DXcall")
End Sub

Private Sub BtnRev_OnLButtonClick()
    Call PostMmttySwitch(m_CurrentSwitch Xor swREV)
End Sub

Private Sub BtnSQ_OnLButtonClick()
    Call PostMmttySwitch(m_CurrentSwitch Xor swSQ)
End Sub

Private Sub btnTU_OnLButtonClick()
    Call SendBtnMsg("btnTU", "%DXcall TU %myCall QRZ?")
End Sub

Private Sub BtnTx_OnLButtonClick()
    Dim message As String
    
    BtnTx.Caption = "Req"
    If BtnTx.State Then
        BtnTx.State = False
    Else
        BtnTx.State = True
    End If
    
    message = "RTTY:{X1}"
    SendMsg (message)
End Sub

Private Sub BtnTxOff_OnLButtonClick()
    Dim message As String
    
    Call SendMsg("RTTY:{X0}{W0}")
    
End Sub

Private Sub BtnType_OnLButtonClick()
    Dim Typ As Long
    Typ = m_CurrentSwitch And 3
    Typ = Typ + 1
    If Typ >= 4 Then Typ = 0
    Typ = Typ + (m_CurrentSwitch And (Not 3))
    Call PostMmttySwitch(Typ)
End Sub

Private Sub BtnUOS_OnLButtonClick()
    Call PostMmttySwitch(m_CurrentSwitch Xor swUOS)
End Sub

Private Sub cmdClosePipe_Click()
    Call CloseHandle(PipeHandle)
    cmdConnect.Caption = "Re-Connect"
End Sub

Private Sub Form_Load()
    SettingsFileName = App.Path + "\RTTY.ini"

    RTTYMain.Left = GetPPS("RTTY", "LeftPos", "100", SettingsFileName)
    RTTYMain.Width = GetPPS("RTTY", "Width", "1000", SettingsFileName)
    RTTYMain.Top = GetPPS("RTTY", "TopPos", "100", SettingsFileName)
    RTTYMain.Height = GetPPS("RTTY", "Height", "1000", SettingsFileName)
    
    AlwaysOnTop RTTYMain
    m_CurrentSwitch = -1
    m_CurrentSwitchReady = 0
    XYscope.HighQuality = KVQ.Checked
    Call UpdateMenus
    XMMR.Item(1).bActive = True
    XMMR.Item(1).bNotifyXY = True
    XMMR.Item(1).bNotifyFFT = True
    Call ShowStatus
    
    txtLoggedCalls.Text = Str$(ReadCallsignsFromLogFile(LogFileName))           'load callsign "database"
End Sub

Private Sub InputInvokeCommand(MsgStr As String)
    Dim InvokeString As String
    InvokeString = InputBox(MsgStr, , XMMR.Item(1).InvokeCommand)
    If Len(InvokeString) Then
        XMMR.Item(1).InvokeCommand = InvokeString
        Call KOR_Click
    End If
End Sub

Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    
    Call CloseHandle(PipeHandle)
    
    'save settings
    If RTTYMain.WindowState = vbNormal Then
        Call WritePPS("RTTY", "LeftPos", RTTYMain.Left, SettingsFileName)
        Call WritePPS("RTTY", "Width", RTTYMain.Width, SettingsFileName)
        Call WritePPS("RTTY", "TopPos", RTTYMain.Top, SettingsFileName)
        Call WritePPS("RTTY", "Height", RTTYMain.Height, SettingsFileName)
    End If
End Sub

Private Sub KOI_Click()
    InputInvokeCommand ("Enter Invoking command:")
End Sub

Private Sub KOR_Click()
    XMMR.Item(1).bActive = False
    XMMR.Item(1).bActive = True
    Call ShowStatus
End Sub

Private Sub KOS_Click()
    Call XMMR.Item(1).PostMmttyMessage(RXM_SHOWSETUP, 0)
End Sub

Private Sub KPD_Click()
    Call XMMR.Item(1).PostMmttyMessage(RXM_PROFILE, MAKELONG(1025, 0))
End Sub

Private Sub KPS_Click(Index As Integer)
    Call XMMR.Item(1).PostMmttyMessage(RXM_PROFILE, MAKELONG(Index, 0))
End Sub

Private Sub KPR_Click()
    Call XMMR.Item(1).PostMmttyMessage(RXM_PROFILE, MAKELONG(0, 3))
End Sub

Private Sub KPW_Click()
    Call XMMR.Item(1).PostMmttyMessage(RXM_PROFILE, MAKELONG(0, 4))
End Sub

Private Sub KVQ_Click()
    XYscope.HighQuality = Not XYscope.HighQuality
    KVQ.Checked = XYscope.HighQuality
End Sub

Private Sub Label1_Click()

End Sub

Private Sub Level_OnLMouseDown(ByVal Level As Integer)
    Call XMMR.Item(1).PostMmttyMessage(RXM_SETSQLVL, Level)
End Sub

Private Sub Level_OnLMouseMove(ByVal Level As Integer)
    Call Level_OnLMouseDown(Level)
End Sub

Private Sub Level_OnLMouseUp(ByVal Level As Integer)
    Call Level_OnLMouseDown(Level)
End Sub

Private Sub Spectram_OnLMouseDown(ByVal Freq As Integer)
    If Freq < 300 Then Freq = 300
    Dim Spacefreq As Long
    Spacefreq = m_nmmr(xr_spacefreq) - m_nmmr(xr_markfreq) + Freq
    If Spacefreq > 2700 Then
        Dim offset As Long
        offset = Spacefreq - 2700
        Spacefreq = Spacefreq - offset
        Freq = Freq - offset
    End If
    Call XMMR.Item(1).PostMmttyMessage(RXM_SETMARK, Freq)
    Call XMMR.Item(1).PostMmttyMessage(RXM_SETSPACE, Spacefreq)
End Sub

Private Sub Spectram_OnLMouseMove(ByVal Freq As Integer)
    Call Spectram_OnLMouseDown(Freq)
End Sub

Private Sub Spectram_OnLMouseUp(ByVal Freq As Integer)
    Call Spectram_OnLMouseDown(Freq)
End Sub

Private Sub Spectram_OnRMouseDown(ByVal Freq As Integer)
    If Freq < 300 Then Freq = 300
    If Freq > 2700 Then Freq = 2700
    Call XMMR.Item(1).PostMmttyMessage(RXM_NOTCH, Freq)
    If (m_CurrentSwitch And (swLMS Or swNOT)) <> (swLMS Or swNOT) Then
        m_nmmr(xr_codeswitch) = m_CurrentSwitch Or swLMS Or swNOT
        Call UpdateButtonState
        Call PostMmttySwitch(m_CurrentSwitch)
    End If
End Sub

Private Sub Spectram_OnRMouseMove(ByVal Freq As Integer)
    If (m_CurrentSwitch And swTWO) = 0 Then Call Spectram_OnRMouseDown(Freq)
End Sub

Private Sub Spectram_OnRMouseUp(ByVal Freq As Integer)
    If (m_CurrentSwitch And swTWO) = 0 Then Call Spectram_OnRMouseDown(Freq)
End Sub


Private Sub Text_Mouseup(Button As Integer, Shift As Integer, X As Single, y As Single)
    ' Only proceed if the left mouse button was released
    If Button = vbLeftButton Then
        Dim WordToCopy As String
        
        ' 1. Get the cursor position (SelStart) where the click occurred
        ' The SelStart property will be automatically set to the click position on MouseUp.
        Dim CursorPos As Long
        CursorPos = Text.SelStart
        
        'If Shift > 0 Then CursorPos = Text.SelStart + Text.SelLength
        


        
        ' 2. Find the start and end of the word at CursorPos
        Dim StartPos As Long
        Dim endpos As Long
        Dim FullText As String
        
        Text.SelStart = 0
        Text.SelLength = Len(Text.Text)
        
        FullText = Text.Text
        Text.SelStart = CursorPos
        Text.SelLength = 0
        
        ' Find the start of the word (look backward for a space or beginning of text)
        StartPos = CursorPos
        If StartPos = 0 Then StartPos = 1
        Do While StartPos > 1 And Mid$(FullText, StartPos, 1) <> " " And Mid$(FullText, StartPos, 1) <> vbCr And Mid$(FullText, StartPos, 1) <> vbLf
            StartPos = StartPos - 1
        Loop
        ' If we stopped on a delimiter or before the start, move one position forward
        If StartPos < CursorPos And (Mid$(FullText, StartPos + 1, 1) = " " Or Mid$(FullText, StartPos + 1, 1) = vbCr Or Mid$(FullText, StartPos + 1, 1) = vbLf) Then
            StartPos = StartPos + 1
        End If
        
        ' Handle the case where the click was on a space at the start of a word
        If StartPos = 0 And Mid$(FullText, 1, 1) = " " Then
            StartPos = 1
        End If

        ' Find the end of the word (look forward for a space or end of text)
        endpos = CursorPos
        If endpos = 0 Then endpos = 1
        Do While endpos <= Len(FullText) And Mid$(FullText, endpos, 1) <> " " And Mid$(FullText, endpos, 1) <> vbCr And Mid$(FullText, endpos, 1) <> vbLf
            endpos = endpos + 1
        Loop
        ' EndPos is now one position after the word ends
        endpos = endpos - 1
        
        ' 3. Extract the word
        Dim WordLength As Long
        If endpos >= StartPos Then
            WordLength = endpos - StartPos
            WordToCopy = Mid$(FullText, StartPos + 1, WordLength)

            ' 4. Select the word (optional, but gives visual feedback)
            'Text.SelStart = StartPos  ' - 1 ' SelStart is 0-based for setting, 1-based for Len/Mid$
            'Text.SelLength = WordLength - 1
            
            ' 5. Copy the selected word to the clipboard
            ' Using Clipboard object is more reliable than Text.Copy in this scenario
            If Len(WordToCopy) > 0 Then
                If Shift = 0 Then
                    txtCallsign.Text = WordToCopy
                ElseIf Shift = 1 Then
                    If txtReportIn.Text = "" Then
                        txtReportIn.Text = WordToCopy
                    Else
                        txtNumberIn = WordToCopy
                    End If
                ElseIf Shift = 2 Then
                    txtLoc.Text = WordToCopy
                ElseIf Shift = 3 Then
                    txtName.Text = WordToCopy
                ElseIf Shift = 4 Then
                    txtQTH.Text = WordToCopy
                End If
                ' You can add a status bar message here for feedback:
                ' Me.StatusBar1.Panels(1).Text = "Copied: " & WordToCopy
            End If
        End If
        Text.SelStart = Len(Text.Text)
        Text.SelLength = 0
    End If
End Sub



Private Sub Timer1_Timer()
    Dim ret As Boolean
    
    If m_CurrentSwitchReady < 3 Then
        Call PostMmttySwitch(m_CurrentSwitch Xor swREV)
        m_CurrentSwitchReady = m_CurrentSwitchReady + 1
    Else
        Timer1.Enabled = False
    End If
    
    If cmdConnect.Caption <> "Connected" Then
        ret = PipeConnect()
        If Not ret Then Timer1.Enabled = True
    End If
End Sub

Private Sub TimerFreq_Timer()
    Dim msg As String
    Dim endpos As Long
    msg = PipeReadStringWithTimeout(hPipe, 1)
    Do While msg <> ""
        If Mid$(msg, 1, 2) = "{F" Then
            endpos = InStr(msg, "}")
            If endpos > 0 Then
                txtFreq.Text = Mid$(msg, 3, endpos - 3) / 1000000
            End If
        End If
        msg = PipeReadStringWithTimeout(hPipe, 1)
    Loop
End Sub

Private Sub txtCallsign_Change()
    If DupeIndex(txtCallsign.Text, txtFreq.Text) < 0 Then
        lblDupe.Caption = ""
    Else
        lblDupe.Caption = "DUPE"
    End If
End Sub

Private Sub txtData_KeyUp(KeyCode As Integer, Shift As Integer)
    If KeyCode = vbKeyReturn Then
        Call cmdSend_Click
    End If
End Sub



Private Sub WaterFall_OnLMouseDown(ByVal Freq As Integer)
    Call Spectram_OnLMouseDown(Freq)
End Sub

Private Sub WaterFall_OnLMouseMove(ByVal Freq As Integer)
    Call Spectram_OnLMouseDown(Freq)
End Sub

Private Sub WaterFall_OnLMouseUp(ByVal Freq As Integer)
    Call Spectram_OnLMouseDown(Freq)
End Sub

Private Sub WaterFall_OnRMouseDown(ByVal Freq As Integer)
    Call Spectram_OnRMouseDown(Freq)
End Sub

Private Sub WaterFall_OnRMouseMove(ByVal Freq As Integer)
    Call Spectram_OnRMouseMove(Freq)
End Sub

Private Sub WaterFall_OnRMouseUp(ByVal Freq As Integer)
    Call Spectram_OnRMouseUp(Freq)
End Sub

Private Sub ShowStatus()
    If XMMR.Item(1).bActive Then
        LStatus.Caption = "Connected to MMTTY " + XMMR.Item(1).verMMTTY
    ElseIf XMMR.Item(1).bInvoking Then
        LStatus.Caption = "Invoking..."
    Else
        LStatus.Caption = "Disconnected"
    End If
End Sub

Private Sub PostMmttySwitch(cs As Long)
    Call XMMR.Item(1).SetMmttySwitch(cs)
End Sub

Private Sub SendButtonMsg(MsgStr As String)
    Call XMMR.Item(1).SendString(MsgStr)
    Call XMMR.Item(1).SetMmttyPTT(1)    'Flush TX buffer and RX
End Sub

Private Sub UpdateButtonState()
    If m_CurrentSwitch <> m_nmmr(xr_codeswitch) Then
        m_CurrentSwitch = m_nmmr(xr_codeswitch)
        BtnUOS.State = m_CurrentSwitch And swUOS
        BtnRev.State = m_CurrentSwitch And swREV
        BtnSQ.State = m_CurrentSwitch And swSQ
        BtnLMS.State = m_CurrentSwitch And swLMS
        BtnBPF.State = m_CurrentSwitch And swBPF
        BtnATC.State = m_CurrentSwitch And swATC
        BtnNET.State = m_CurrentSwitch And swNET
        BtnAFC.State = m_CurrentSwitch And swAFC
        If (m_CurrentSwitch And swNOT) Then BtnLMS.Caption = "NOT" Else BtnLMS.Caption = "LMS"
        Select Case (m_CurrentSwitch And 3)
            Case 0
                BtnType.Caption = "IIR"
            Case 1
                BtnType.Caption = "FIR"
            Case 2
                BtnType.Caption = "PLL"
            Case Else
                BtnType.Caption = "FFT"
        End Select
        If m_CurrentSwitchReady = 0 Then m_CurrentSwitchReady = 1
        
    End If
End Sub

Private Sub AddChar(strChar As String)
    If strChar = vbCr Then strChar = strChar & vbLf
    Text.SelStart = Len(Text.Text)
    Text.SelLength = 0
    Text.SelText = strChar
    Text.SelStart = Len(Text.Text)
    Text.SelLength = 0
End Sub


Private Sub XMMR_OnCharRcvd(Index As Integer, ByVal bChar As Integer)
    If bChar <> 10 Then
        AddChar (Chr$(bChar))
    End If
End Sub



Private Sub XMMR_OnConnected(Index As Integer)
    XYscope.DemSampleFreq = XMMR.Item(1).smpDemFreq
    Call ShowStatus
    Call UpdateMenus
End Sub


Private Sub XMMR_OnDisconnected(Index As Integer, ByVal status As Integer)
    Select Case status
    Case 0
        LStatus.Caption = "MMTTY closing failed."
    Case 2
        LStatus.Caption = "MMTTY invoking failed."
        InputInvokeCommand ("Probably, MMTTY was not invoked." _
                            + Chr(13) + Chr(10) + Chr(13) + Chr(10) _
                            + "Enter full path name." _
                            + Chr(13) + Chr(10) _
                            + "Or copy mmtty.exe and userpara.ini to the current folder.")
    End Select
End Sub


Private Sub XMMR_OnNotifyFFT(Index As Integer, pFFT As Integer, ByVal size As Integer, ByVal sampfreq As Integer)
    Call Spectram.Draw(pFFT, size, sampfreq)
    Call WaterFall.Draw(pFFT, size, sampfreq)
End Sub


Private Sub XMMR_OnNotifyNMMR(Index As Integer, pNMMR As Long)
    Call CopyMemory(m_nmmr(0), pNMMR, 64 * 4)
    Call Level.DrawByNMMR(pNMMR)
    Call Spectram.UpdateByNMMR(pNMMR)
    Call UpdateButtonState
    If (m_nmmr(xr_codeview) And &H1000) = 0 Then
        Call XMMR.Item(1).SetMmttyView(m_nmmr(xr_codeview) Or &H1000)
    End If
End Sub


Private Sub XMMR_OnNotifyXY(Index As Integer, pXY As Long)
    Call XYscope.Draw(pXY)
End Sub


Private Sub XMMR_OnPttEvent(Index As Integer, ByVal btx As Integer)
    BtnTx.State = btx <> 0
    BtnTx.Caption = "TX"
End Sub

Private Sub UpdateMenus()
    Dim i As Integer
    For i = 0 To PROFILEEND Step 1
        KPS(i).Caption = "&" + CStr(i + 1) + " " + XMMR.Item(1).Profiles(i)
        KPS(i).Enabled = Len(XMMR.Item(1).Profiles(i)) <> 0
    Next i
    Dim bFlag As Boolean
    bFlag = XMMR.Item(1).bActive
    KPR.Enabled = bFlag
    'KPD.Enabled = bFlag
    KPW.Enabled = bFlag
    KOS.Enabled = bFlag
End Sub


'*******************************************************************
'*****   PIPE COMMS ************************************************
'*******************************************************************

Private Sub cmdConnect_Click()
    Dim ret As Boolean
    ret = PipeConnect()
    
    If Not ret Then Timer1.Enabled = True       'try again
    
End Sub

Private Function PipeConnect() As Boolean
    Dim lSuccess As Long

    ' 1. Wait up to 5 seconds for the pipe server to be ready
    lSuccess = WaitNamedPipe(PIPE_NAME, 2000)
    
    If lSuccess = 0 Then
        ' Pipe is not available after 5 seconds (Error 2 or Timeout)
        'MsgBox "Pipe is not available. Ensure C++ Server is running.", vbCritical
        PipeConnect = False
        Exit Function
    End If
    
    ' 2. Attempt to open (connect)
    hPipe = CreateFile( _
        PIPE_NAME, _
        GENERIC_READ Or GENERIC_WRITE, _
        0, _
        ByVal 0&, _
        OPEN_EXISTING, _
        0, _
        0)

    If hPipe = INVALID_HANDLE_VALUE Then
        MsgBox "Failed to connect to pipe: " & Err.LastDllError, vbCritical
        cmdConnect.Caption = "Connect Failed"
        PipeHandle = 0
        PipeConnect = False
    Else
        'MsgBox "Connected to C++ Pipe Server!", vbInformation
        cmdConnect.Caption = "Connected"
        PipeHandle = hPipe
        PipeConnect = True
    End If
End Function

Private Sub cmdSend_Click()
    Call SendBtnMsg("cmdSend", txtData.Text)
End Sub

Private Sub SendBtnMsg(btnName As String, strDefault As String)
'read btn string from ini file, construct tags and send out

    Dim myCall As String
    Dim myName As String
    Dim myQTH As String
    Dim myLoc As String
    
    Dim btnStr As String
    Dim tmp As String
    
    myCall = GetPPS("RTTY", "myCall", "", SettingsFileName)
    If myCall = "" Then
        myCall = "G4AHN"
        Call WritePPS("RTTY", "myCall", myCall, SettingsFileName)
    End If
    
    myName = GetPPS("RTTY", "myName", "", SettingsFileName)
    If myName = "" Then
        myName = "RICH"
        Call WritePPS("RTTY", "myName", myName, SettingsFileName)
    End If

    myQTH = GetPPS("RTTY", "myQTH", "", SettingsFileName)
    If myQTH = "" Then
        myQTH = "NR LONDON"
        Call WritePPS("RTTY", "myQTH", myQTH, SettingsFileName)
    End If
    
    myLoc = GetPPS("RTTY", "myLOC", "", SettingsFileName)
    If myLoc = "" Then
        myLoc = "IO91OE"
        Call WritePPS("RTTY", "myLOC", myLoc, SettingsFileName)
    End If

    If btnName <> "cmdSend" Then             'exclude special case(s) from ini file
        btnStr = GetPPS("RTTY", btnName, "", SettingsFileName)
        If btnStr = "" Then
            Call WritePPS("RTTY", btnName, strDefault, SettingsFileName)
            btnStr = strDefault
        End If
        
        If chkEdit.Value = vbChecked Then
            tmp = "% to reset to default" & vbCrLf + "%myCall %myName %myQTH %myLOC" + vbCrLf + "%DXcall %DXloc" + vbCrLf + "%RSTout %NumOut %RSTin %NumIn"
            btnStr = InputBox(tmp, "Edit Button Message", btnStr)       'returns "" if cancelled
            If btnStr <> "" Then Call WritePPS("RTTY", btnName, btnStr, SettingsFileName)
            If btnStr = "%" Then Call WritePPS("RTTY", btnName, strDefault, SettingsFileName)
            chkEdit.Value = vbUnchecked
            Exit Sub
        End If
        
    Else
        btnStr = strDefault
    End If

    btnStr = Replace(btnStr, "%myCall", myCall)
    btnStr = Replace(btnStr, "%DXcall", txtCallsign)
    btnStr = Replace(btnStr, "%myName", myName)
    btnStr = Replace(btnStr, "%myQTH", myQTH)
    btnStr = Replace(btnStr, "%myLOC", myLoc)
    
    btnStr = Replace(btnStr, "%RSTout", txtReportOut)
    btnStr = Replace(btnStr, "%NumOut", txtNumberOut)
    btnStr = Replace(btnStr, "%RSTin", txtReportIn)
    btnStr = Replace(btnStr, "%NumIn", txtNumberIn)
    
    btnStr = Replace(btnStr, "%DXloc", txtLoc)
    btnStr = Replace(btnStr, "%DXname", txtName)
    btnStr = Replace(btnStr, "%DXqth", txtQTH)
    
    btnStr = "RTTY:{M1}{Z" + btnStr + "}{X1}"
    Call SendMsg(btnStr)

End Sub


Private Sub SendMsg(message As String)
    Dim sData As String
    Dim lBytesWritten As Long
    Dim lSuccess As Long
    
    If hPipe = 0 Or hPipe = INVALID_HANDLE_VALUE Then
        cmdConnect.Caption = "Re-Connect Pipe"
        MsgBox "Pipe is not connected.", vbExclamation
        Exit Sub
    End If
    
    sData = message & Chr$(0) ' Append null terminator for C/C++ compatibility
    
    ' Write the data to the pipe
    lSuccess = WriteFile(hPipe, ByVal StrPtr(sData), LenB(sData), lBytesWritten, 0)
    
    If lSuccess = 0 Then
        cmdConnect.Caption = "Re-Connect Pipe"
        MsgBox "Write failed. Error: " & Err.LastDllError, vbCritical
    'Else
        'MsgBox "Sent " & lBytesWritten & " bytes to C++ Server.", vbInformation
    End If
End Sub

Public Function PipeReadStringWithTimeout(ByVal hPipe As Long, ByVal lTimeoutMs As Long) As String
    
    Dim lBytesToRead As Long
    Dim lTotalBytesAvail As Long
    Dim lBytesLeftThisMessage As Long
    Dim lSuccess As Long
    Dim lBytesRead As Long
    Dim sBuffer As String
    
    ' Set a minimum timeout, as 0 can be problematic.
    If lTimeoutMs = 0 Then lTimeoutMs = 1
    
    ' Step 1: Wait for data (or pipe availability) with the specified timeout.
    ' Note: If the pipe handle is already open and connected, this will often succeed immediately.
    ' If the pipe is disconnected and needs to reconnect, this is more useful.
    ' For strictly message availability, PeekNamedPipe is better.
    
    ' Step 2: Use PeekNamedPipe to check for data and get the message size.
    ' We pass 0 as the buffer size since we only want to peek at the message length.
    lSuccess = PeekNamedPipe(hPipe, ByVal 0&, 0, lBytesRead, lTotalBytesAvail, lBytesLeftThisMessage)
    
    If lSuccess <> 0 Then
        ' Check if a full message is available.
        If lBytesLeftThisMessage > 0 Then
            ' The variable lBytesLeftThisMessage holds the exact size of the current message.
            lBytesToRead = lBytesLeftThisMessage
            
            ' Resize the string buffer to hold the incoming data + 1 for null termination
            ' (String functions will handle the conversion/null termination on read).
            sBuffer = String$(lBytesToRead, Chr$(0))
            
            ' Step 3: Read the message synchronously.
            lSuccess = ReadFile(hPipe, ByVal StrPtr(sBuffer), lBytesToRead, lBytesRead, ByVal 0&)
            
            If lSuccess <> 0 And lBytesRead = lBytesToRead Then
                ' Return the read string, trimmed to the actual bytes read.
                PipeReadStringWithTimeout = Left$(sBuffer, lBytesRead)
                Exit Function
            Else
                ' ReadFile failed (e.g., pipe disconnected unexpectedly)
                ' Consider logging the error via Err.LastDllError
                PipeReadStringWithTimeout = ""
                Exit Function
            End If
        End If
    Else
        ' PeekNamedPipe failed (e.g., pipe closed, or other error).
        ' For timeout behavior, we just return "".
    End If
    
    ' If no message was available after peeking, return an empty string.
    PipeReadStringWithTimeout = ""
End Function

Private Sub Form_Unload(Cancel As Integer)
    ' Always close the handle when the form unloads
    If hPipe <> 0 And hPipe <> INVALID_HANDLE_VALUE Then
        CloseHandle hPipe
    End If
End Sub

' NOTE: Reading data is more complex, typically requiring a dedicated loop
' or a timer to poll for data, as Named Pipes are blocking by default.
' You'd use ReadFile() in a similar way to WriteFile().


