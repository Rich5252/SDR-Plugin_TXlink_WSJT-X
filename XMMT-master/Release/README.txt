This document contains two main parts: a **software license notice** and a **description of the project files** for an ActiveX control DLL.

## Software License Notice (LGPL)

```
//Copyright LGPL

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000-2013 Makoto Mori, Nobuyuki Oba
//-----------------------------------------------------------------------------------------------------------------------------------------------
// This file is part of MMTTY.

// MMTTY is free software: you can redistribute it and/or modify it under the terms of the **GNU Lesser General Public License**
// as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// MMTTY is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY**; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the **GNU Lesser General Public License** for more details.

// You should have received a copy of the GNU Lesser General Public License along with MMTTY. If not, see 
// <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------------------------------------------------------------------------
```

-----

## ActiveX Control DLL: XMMT Project File Description

```
========================================================================
		ActiveX Control DLL : XMMT
========================================================================
```

ControlWizard created this project for the **XMMT ActiveX control DLL**, which includes **3 controls**.

This skeleton project not only demonstrates the basics of creating an ActiveX control but can also be used as a template for creating specific control functionalities.

This file contains an overview of the individual files that constitute the XMMT ActiveX control DLL.

### Main DLL Files

| Filename | Description |
| :--- | :--- |
| **XMMT.dsp** | This file (the **project file**) contains project-level information and is used for building a single project or sub-projects. You can share the project file (.dsp) with other users, but the make file should be exported locally. |
| **XMMT.h** | This file is the central **include file** for the ActiveX control DLL. It also includes other project-specific include files, such as `resource.h`. |
| **XMMT.cpp** | This file is the main DLL **source file**, containing DLL initialization, termination, and other definitions. |
| **XMMT.rc** | This file describes all the **Microsoft Windows resources** used in the project. It can be edited directly with the Visual C++ Resource Editor. |
| **XMMT.def** | This file contains the information necessary for the ActiveX control DLL to run on **Microsoft Windows**. |
| **XMMT.clw** | This file holds information used by **ClassWizard** when editing existing classes or creating new ones. It also contains information necessary for ClassWizard to edit or generate message maps and dialog data maps, and to generate prototype member functions. |
| **XMMT.odl** | This file contains the **Object Description Language** source code for the controls' type library. |

-----

### XMMT Control Files

| Filename | Description |
| :--- | :--- |
| **XMMTCtl.h** | This file contains the **declaration** of the `CXMMTCtrl` C++ class. |
| **XMMTCtl.cpp** | This file contains the **implementation** of the `CXMMTCtrl` C++ class. |
| **XMMTPpg.h** | This file contains the **declaration** of the `CXMMTPropPage` C++ class. |
| **XMMTPpg.cpp** | This file contains the **implementation** of the `CXMMTPropPage` C++ class. |
| **XMMTCtl.bmp** | This file contains the **bitmap** that a container uses to display the `CXMMTCtrl` control when it appears in a tool palette. This bitmap is included by the main resource file, `XMMT.rc`. |

-----

### XMMXY Control Files

| Filename | Description |
| :--- | :--- |
| **XMMXYCtl.h** | This file contains the **declaration** of the `CXMMXYCtrl` C++ class. |
| **XMMXYCtl.cpp** | This file contains the **implementation** of the `CXMMXYCtrl` C++ class. |
| **XMMXYPpg.h** | This file contains the **declaration** of the `CXMMXYPropPage` C++ class. |
| **XMMXYPpg.cpp** | This file contains the **implementation** of the `CXMMXYPropPage` C++ class. |
| **XMMXYCtl.bmp** | This file contains the **bitmap** that a container uses to display the `CXMMXYCtrl` control when it appears in a tool palette. This bitmap is included by the main resource file, `XMMT.rc`. |

-----

### XMMBtn Control Files

| Filename | Description |
| :--- | :--- |
| **XMMBtnCtl.h** | This file contains the **declaration** of the `CXMMBtnCtrl` C++ class. |
| **XMMBtnCtl.cpp** | This file contains the **implementation** of the `CXMMBtnCtrl` C++ class. |
| **XMMBtnPpg.h** | This file contains the **declaration** of the `CXMMBtnPropPage` C++ class. |
| **XMMBtnPpg.cpp** | This file contains the **implementation** of the `CXMMBtnPropPage` C++ class. |
| **XMMBtnCtl.bmp** | This file contains the **bitmap** that a container uses to display the `CXMMBtnCtrl` control when it appears in a tool palette. This bitmap is included by the main resource file, `XMMT.rc`. |

-----

### Other Standard Files

| Filename(s) | Description |
| :--- | :--- |
| **stdafx.h, stdafx.cpp** | These files are used to build the **precompiled header file** `stdafx.pch` (PCH) and the precompiled type file (PCT). |
| **resource.h** | This file is the standard header file that defines new **resource IDs**. The Visual C++ Resource Editor reads and updates this file. |

-----

### Other Notes

Comments starting with **"TODO:"** indicate places where source code needs to be added or customized.