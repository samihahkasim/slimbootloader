/**@file
  ACPI DSDT table for MEI1

  Copyright (c) 2024, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
External(HIDW, MethodObj)
External(HIWC, MethodObj)

  //
  // Management Engine Interface 1 - Device 22, Function 0
  //
  Device(HECI) {
    Name(_ADR, 0x00160000)

    Method(_DSM, 0x4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj}) {
      if(PCIC(Arg0)) { return(PCID(Arg0,Arg1,Arg2,Arg3)) }

      //-------------------------------------------
      //  HID Wake up Event solution
      //-------------------------------------------
      If(CondRefOf(HIWC)) {
        If(HIWC(Arg0)) {
          If(CondRefOf(HIDW)) {
            Return (HIDW(Arg0, Arg1, Arg2, Arg3))
          }
        }
      }

      Return(Buffer() {0})
    } // End _DSM
  } // Device(HECI)

  //
  // Management Engine Interface 3 - Device 22, Function 4
  //
  Device(HEC3) {
    Name(_ADR, 0x00160004)

    Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj}) {
      If (PCIC(Arg0)) { Return(PCID(Arg0,Arg1,Arg2,Arg3)) }

      Return(Buffer() {0})
    } // End _DSM
  } // End Device HEC3

