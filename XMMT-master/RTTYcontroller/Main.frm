VERSION 5.00
Begin VB.Form Main 
   BorderStyle     =   1  
   Caption         =   "XMMR 2 Container"
   ClientHeight    =   5175
   ClientLeft      =   150
   ClientTop       =   840
   ClientWidth     =   7395
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5175
   ScaleWidth      =   7395
   StartUpPosition =   3  'Windows
   Begin VB.TextBox Text 
      Height          =   1695
      Index           =   1
      Left            =   0
      MultiLine       =   -1  'True
      ScrollBars      =   2  '
      TabIndex        =   24
      Top             =   3480
      Width           =   7455
   End
   Begin XMMTLib.XMMR XMMR 
      Height          =   255
      Index           =   0
      Left            =   6480
      TabIndex        =   22
      Top             =   1320
      Visible         =   0   'False
      Width           =   240
      _Version        =   65538
      _ExtentX        =   423
      _ExtentY        =   450
      _StockProps     =   0
      bInitialClose   =   0   'False
      bNotifyFFT      =   0   'False
      bNotifyXY       =   0   'False
      bNotifyComChange=   0   'False
      InvokeCommand   =   "MMTTY.EXE -m -Z"
      Title           =   "XMMR/MMTTY engine"
   End
   Begin XMMTLib.XMMBtn BtnM4 
      Height          =   375
      Left            =   1560
      TabIndex        =   21
      Top             =   1320
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
      Caption         =   "M4"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnM3 
      Height          =   375
      Left            =   1080
      TabIndex        =   20
      Top             =   1320
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
      Caption         =   "M3"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnM2 
      Height          =   375
      Left            =   600
      TabIndex        =   19
      Top             =   1320
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
      Caption         =   "M2"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnM1 
      Height          =   375
      Left            =   120
      TabIndex        =   18
      Top             =   1320
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
      Caption         =   "M1"
      FaceColor       =   -2147483633
   End
   Begin VB.TextBox Text 
      Height          =   1695
      Index           =   0
      Left            =   0
      MultiLine       =   -1  'True
      ScrollBars      =   2  '
      TabIndex        =   17
      Top             =   1800
      Width           =   7455
   End
   Begin XMMTLib.XMMBtn BtnAFC 
      Height          =   375
      Left            =   1920
      TabIndex        =   15
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
      TabIndex        =   14
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
      TabIndex        =   13
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
      TabIndex        =   12
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
      Caption         =   "LMS"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnSQ 
      Height          =   375
      Left            =   960
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
      Caption         =   "SQ"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnHAM 
      Height          =   375
      Left            =   1920
      TabIndex        =   9
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
      Caption         =   "Rev"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnType 
      Height          =   375
      Left            =   960
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
      Toggled         =   0   'False
      Caption         =   "Typ"
      FaceColor       =   -2147483633
   End
   Begin XMMTLib.XMMBtn BtnUOS 
      Height          =   375
      Left            =   120
      TabIndex        =   6
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
      TabIndex        =   5
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
      TabIndex        =   4
      Top             =   840
      Width           =   3495
      _Version        =   65538
      _ExtentX        =   6165
      _ExtentY        =   661
      _StockProps     =   32
      BorderStyle     =   1
      BaseFreq        =   250
      WidthFreq       =   2500
   End
   Begin XMMTLib.XMMSpec Spectram 
      Height          =   735
      Left            =   2760
      TabIndex        =   3
      Top             =   120
      Width           =   3495
      _Version        =   65538
      _ExtentX        =   6165
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
      BaseFreq        =   250
      WidthFreq       =   2500
   End
   Begin XMMTLib.XMMXY XyScope 
      Height          =   1095
      Left            =   6240
      TabIndex        =   2
      Top             =   120
      Width           =   1095
      _Version        =   65538
      _ExtentX        =   1931
      _ExtentY        =   1931
      _StockProps     =   32
      BorderStyle     =   1
   End
   Begin XMMTLib.XMMBtn BtnTxOff 
      Height          =   375
      Left            =   120
      TabIndex        =   1
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
      TabIndex        =   0
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
      Left            =   6720
      TabIndex        =   23
      Top             =   1320
      Visible         =   0   'False
      Width           =   240
      _Version        =   65538
      _ExtentX        =   423
      _ExtentY        =   450
      _StockProps     =   0
      bInitialClose   =   0   'False
      bNotifyFFT      =   0   'False
      bNotifyXY       =   0   'False
      bNotifyComChange=   0   'False
      InvokeCommand   =   "MMTTY.EXE -m -Z"
      Title           =   "XMMR/MMTTY engine"
   End
   Begin VB.Label LStatus 
      Caption         =   "Status"
      Height          =   255
      Left            =   2520
      TabIndex        =   16
      Top             =   1320
      Width           =   3615
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
Attribute VB_Name = "Main"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

