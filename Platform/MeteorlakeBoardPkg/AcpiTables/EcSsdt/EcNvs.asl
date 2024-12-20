/**@file

  Copyright (c) 2024, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
  // Define a Global region of ACPI NVS Region that may be used for any
  // type of implementation.  The starting offset and size will be fixed
  // up by the System BIOS during POST.  Note that the Size must be a word
  // in size to be fixed up correctly.

  OperationRegion (ENVS, SystemMemory, 0xFFFF0000, 0xAA55)
  Field (ENVS, AnyAcc, Lock, Preserve)
  {  Offset(0),      IUBE, 8,  // Offset(0),     IUER Button Enable
  Offset(1),      IUCE, 8,  // Offset(1),     IUER Convertible Enable
  Offset(2),      IUDE, 8,  // Offset(2),     IUER Dock Enable
  Offset(3),      ECNO, 8,  // Offset(3),     EC Notification of Low Power S0 Idle State
  Offset(4),      ECLP, 8,  // Offset(4),     EC Low Power Mode: 1 - Enabled, 0 - Disabled
  Offset(5),      BATS, 8,  // Offset(5),     Battery Support - Bit0: Real Battery is supported on this platform. Bit1: Virtual Battery is supported on this platform.
  Offset(6),      EHK3, 8,  // Offset(6),     Ec Hotkey F3 Support
  Offset(7),      EHK4, 8,  // Offset(7),     Ec Hotkey F4 Support
  Offset(8),      EHK5, 8,  // Offset(8),     Ec Hotkey F5 Support
  Offset(9),      EHK6, 8,  // Offset(9),     Ec Hotkey F6 Support
  Offset(10),     EHK7, 8,  // Offset(10),    Ec Hotkey F7 Support
  Offset(11),     EHK8, 8,  // Offset(11),    Ec Hotkey F8 Support
  Offset(12),     VBVP, 8,  // Offset(12),    Virtual Button Volume Up Support
  Offset(13),     VBVD, 8,  // Offset(13),    Virtual Button Volume Down Support
  Offset(14),     VBHB, 8,  // Offset(14),    Virtual Button Home Button Support
  Offset(15),     VBRL, 8,  // Offset(15),    Virtual Button Rotation Lock Support
  Offset(16),     SMSS, 8,  // Offset(16),    Slate Mode Switch Support
  Offset(17),     ADAS, 8,  // Offset(17),    Ac Dc Auto Switch Support
  Offset(18),     PPBG, 32, // Offset(18),    Pm Power Button Gpio Pin
  Offset(22),     EGPE, 32, // Offset(22),    Ecdt GPE bit value
  Offset(26),     LSWP, 32, // Offset(26),    Lid Switch Wake Gpio
  Offset(30),     PGED, 8,  // Offset(30),    Pseudo G3 counter Enable/Disable
  Offset(31),     EBTP, 8,  // Offset(31),    Enable battery _BTP support
  }