/**@file
  Copyright (c) 2024, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

External (\_SB.HDAS_PATH.PPMS, MethodObj)

Device (IDA) {
Name (_ADR, 0x10000000)  // _ADR: Address

Method(_DSM, 0x4, NotSerialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj}) {
    // Arg0 - UUID: A69F886E-6CEB-4594-A41F-7B5DCE24C553 (Buffer)
    // Arg1 - Revision ID: 0x01 (Integer)
    // Arg2 - Function Index: 0x0 - 0x3 and 0x6 (Integer) - See below for details.
    // Arg3 - Depends on Function Index - See below for details.
    // Return - Depends on Function Index - See below for details.

    ADBG("HDAS _DSM")
    ADBG(Arg2)

    if(PCIC(Arg0)) { return(PCID(Arg0,Arg1,Arg2,Arg3)) }

    // Verify UUID
    If (LEqual(Arg0, ToUUID ("A69F886E-6CEB-4594-A41F-7B5DCE24C553"))) {

    Switch(ToInteger(Arg2)) {

        // Function 0: Function Support Query
        // Arg2 - Function Index: 0x00 (Integer)
        // Arg3: Unused
        // Return: Bitmask of functions supported. (Buffer)
        Case(0) {
        // Supports function 0 - 3 and 6
        Return(Buffer(One) { 0xCF })
        }

        // Function 1: Query Non HD Audio Descriptor Table
        //                    Used by the Intel Offload Engine Driver to discover the
        //                    non HD Audio devices supported by the Audio DSP.
        // Arg2 - Function Index: 0x01 (Integer)
        // Arg3 - Unused
        // Return - ACPI Table describing the non HD Audio links and devices supported by the ADSP (ResourceBuffer)
        Case (1) {
            ADBG ("_DSM Fun 1 NHLT")
            // NBUF - Memory Resource Descriptor buffer with address and length of NHLT
            Switch (ToInteger (Arg3)) {
            Case (0) {Return (NBU0)}
            Default {Return (NBU0)}
            } // End of Switch
        }

        // Function 2: Query Feature Mask
        //                    Used by the Intel Offload Engine Driver to retrieve a bitmask
        //                    of features allowable on this platform.
        // Arg2 - Function Index: 0x02 (Integer)
        // Arg3: Unused
        // Return: Bitmask of supported features.
        Case (2) {
        ADBG("_DSM Fun 2 FMSK")
        // Bit 0 == '1', WoV is supported, Bit 0 == '0', WoV not supported
        // Bit 1 == '1', BT Sideband is supported, Bit 1 == '0', BT not supported
        // Bit 2 == '1', codec based VAD support allowable
        // Bit 3 - 4 Reserved
        // Bit 5 == '1', BT Intel HFP SCO is supported
        // Bit 6 == '1', BT Intel A2DP is supported
        // Bit 7 == '1', DSP based speech pre-processing disabled
        // Bit 8 == '1', Windows Voice Activation, Bit 8 == '0', Intel Wake on Voice
        // Bit 9 == '1', BT Intel Low Energy is supported
        // Bit 10 - 31 Reserved, shall be set to '0'
        // ADFM - NVS AudioDSP Feature Bit Mask updated from PchPolicy
        Return(AUDIO_FEATURE_MASK_NVS_PATH)
        }

        // Function 3: Query Pre/Post Processing Module Support
        //                    Used by the Intel Offload Engine Driver to determine if a
        //                    specified PP Module is allowed to be supported on this platform
        // Arg2 - Function Index: 0x03 (Integer)
        // Arg3 - UUID: Specifies the UUID of the PP module to check (Buffer)
        // Return - TRUE if PP Module supported, else FALSE.
        Case (3) {
            ADBG("_DSM Fun 3 PPMS")

            If (CondRefOf (\_SB.HDAS_PATH.PPMS)) { // Call platform method PPMS to check if 3rd Party IP module with given GUID (Arg3) is supported
                Return(\_SB.HDAS_PATH.PPMS (Arg3))
            } Else {
                ADBG("PPMS not found")
            }

            Return(0) // Is not supported
        }
        Case (6) {
            ADBG("_DSM Fun 6 XCFG")
            Return (XTAL_NVS_PATH)
        }
        Case (7) {
            ADBG("_DSM Fun 7 HDBO")
            ADBG(HDBO)

            Return (HDBO)
        }
        Default {
            // Function not supported (Arg2)
            ADBG("_DSM Fun NOK")
            Return(Buffer(One) { 0x00 })
        }
    } // Switch(Arg2) End
    } // If(Arg0, UUID) End

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

    // UUID not supported (Arg0)
    ADBG("_DSM UUID NOK")
    Return(Buffer() {0})
} // _DSM End
}