/** @file
  ACPI uPEP Support for PCIe RP Endpoint Type checking

  Copyright (c) 2024, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

  // Include PepPcieEndpoint.asl for PCIe RP PEP support
  // Input parameters:
  // PERN - PCIe Root Port Name String (For Debug Purpose)

  External (WIST, MethodObj)
  External (WWST, MethodObj)

  External (BCCX)
  External (SCCX)
  External (PIXX)
  External (ISGX, MethodObj)
  External (PNVM, MethodObj)
  External (PAHC, MethodObj)
  External (PRES, MethodObj)

  //
  // Check if EP(End Point) is LAN
  // Arguments: (0)
  // Return:
  //     0->EP is not LAN; 1->EP is LAN
  //
  Method (ISLN, Zero, Serialized) // Check if PCIe LAN device
  {
    If (LEqual (BCCX, 0x02)) {     // Check Sub Class Code and Base Class Code
      If (LEqual (SCCX, 0x00)) {
        If (LEqual (PIXX, 0x00)) {
          ADBG ("PCIe LAN")
          Return (0x01)
        }
      }
    }
    Return (0x00)
  }

  //
  // Check if EP(End Point) is DTBT RP
  // Arguments: (0)
  // Return:
  //     0->EP is not DTBT RP; 1->EP is DTBT RP
  //
  Method (ISDT, Zero, Serialized) // Check if PCH PCIe DTBT RP
  {
    If (CondRefOf (\DTFS)) {
      If (LAnd (LEqual (\DTFS, 0x01), LOr ( LAnd (LEqual (\RPS0, SLOT), \RPN0), LAnd (LEqual (\RPS1, SLOT), \RPN1)))) {
        ADBG ( Concatenate( "DTBT PEP Constraint is successfully SET for PCH RP = ", ToDecimalString (SLOT)))
        Return (0x01)
      }
    }
    Return (0x00)
  }

  //
  // Get EP (End Point) device type
  // Arguments: (0)
  // Return:
  //         0->Other
  //         1->Storage
  //         2->LAN - PCH
  //         3->WLAN - PCH
  //         4->GFX -  CPU, PCH
  //         5->DTBT - PCH
  //         6->WWAN - PCH
  //         99->Invalid (EP absent)
  //
  Method (GRPT, Zero, Serialized) {

    ADBG (Concatenate ("Root Port : ", PERN))

     If (CondRefOf (^^PRMV)) {
       If (LEqual (^^PRMV, 1)) {
         ADBG (Concatenate (PERN," mapped under VMD"))
         Return (99);
       }
     }

     If (PRES ()) {
       If (LOr (PAHC (), PNVM ())) {
         Return (1);
       }

      If (ISGX ()) {
        Return (4);
      }

      If (LEqual (PRTP, PCIE_RP_TYPE_PCH)) { // Currently Only Following devices are supported by PCH PCIE RP
        If (ISLN ()) {
          Return (2);
        }

       If (CondRefOf (^WIST)) {//only PCH RP has wifi controller support
         If (^WIST ()) {
           Return (3);
         }
       }

        If (ISDT ()) {
          Return (5);
        }

       If (CondRefOf (WWST)) {
          If (WWST ()) {
            Return (6);
          }
        }
      }

      Return(0);
    }

     Return (99);
  }
