/** @file

  Copyright (c) 2024, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

//
// Defined as an SSDT to be able to dynamically load based on BIOS
// setup options
//
#include <Include/AcpiDebug.h>

DefinitionBlock (
  "THERMAL.aml",
  "SSDT",
  0x02,
  "TherRv",
  "Ther_Rvp",
  0x1000
  )
{

ACPI_DEBUG_EXTERNAL_REFERENCE
External (\_SB.APSV)
External (\_SB.ACRT)
External (\_SB.AAC0)
External (\_SB.PR00)
External (\_SB.PR01)
External (\_SB.PR02)
External (\_SB.PR03)
External (\_SB.PR04)
External (\_SB.PR05)
External (\_SB.PR06)
External (\_SB.PR07)
External (\_SB.PR08)
External (\_SB.PR09)
External (\_SB.PR10)
External (\_SB.PR11)
External (\_SB.PR12)
External (\_SB.PR13)
External (\_SB.PR14)
External (\_SB.PR15)
External (\_SB.PR16)
External (\_SB.PR17)
External (\_SB.PR18)
External (\_SB.PR19)
External (\_SB.PR20)
External (\_SB.PR21)
External (\_SB.PR22)
External (\_SB.PR23)
External (\_SB.PR24)
External (\_SB.PR25)
External (\_SB.PR26)
External (\_SB.PR27)
External (\_SB.PR28)
External (\_SB.PR29)
External (\_SB.PR30)
External (\_SB.PR31)
External (\_SB.PR32)
External (\_SB.PR33)
External (\_SB.PR34)
External (\_SB.PR35)
External (\_SB.PR36)
External (\_SB.PR37)
External (\_SB.PR38)
External (\_SB.PR39)
External (\_SB.PR40)
External (\_SB.PR41)
External (\_SB.PR42)
External (\_SB.PR43)
External (\_SB.PR44)
External (\_SB.PR45)
External (\_SB.PR46)
External (\_SB.PR47)

External(\CTYP, IntObj)
External(\TCNT, IntObj)
External(\VFN0, IntObj)
External(\VFN1, IntObj)
External(\VFN2, IntObj)
External(\VFN3, IntObj)
External(\VFN4, IntObj)
External(\ECON, IntObj)
External(\AC0F, IntObj)
External(\AC1F, IntObj)
External(\CRTT, IntObj)
External(\PSVT, IntObj)
External(\ACTT, IntObj)
External(\ACT1, IntObj)
External(\TC1V, IntObj)
External(\TC2V, IntObj)
External(\TSPV, IntObj)

External (\_SB.DPTF, IntObj)
External (\_SB.FNAU, IntObj)

External (\_SB.PC00.LPCB.MXTP, MethodObj)
External (\_SB.PC00.LPCB.UPFS, MethodObj)
External(\_SB.PC00.LPCB.ITE8.GETT, MethodObj)
External(\_SB.PC00.LPCB.ITE8.STFN, MethodObj)
External(\_SB.PC00.LPCB.ITE8.GFNL, MethodObj)
External(\_SB.PC00.LPCB.ITE8.GFNH, MethodObj)


// THERMAL.ASL represents a Thermal Zone to be used for testing on the
// Customer Reference Boards.
Scope(\_TZ)
{

  // Notes:
  //  1) WIN2K strictly uses interrupt driven thermal events.
  //  2) Temperature values are stored in tenths of Kelvin to
  //    eliminate the decimal place.
  //  3) Kelvin = Celsius + 273.2.
  //  4) All temperature must be >= 289K.

  Name(ETMD, 1)

  //Fan Control Event
  Event(FCET)

  //Fan Control Running
  Name(FCRN, 0)
  Mutex(FCMT, 0)

  //Cached Virtual Fan Status
  Name(CVF0, 0)
  Name(CVF1, 0)
  Name(CVF2, 0)
  Name(CVF3, 0)
  Name(CVF4, 0)

  //Cached Virtual Fan Status Mutexes
  Mutex(FMT0, 0)
  Mutex(FMT1, 0)
  Mutex(FMT2, 0)
  Mutex(FMT3, 0)
  Mutex(FMT4, 0)

  // Fan 0 = Package Processor Fan - Maximum speed
  PowerResource(FN00, 0, 0)
  {
    Method(_STA, 0, Serialized)
    {
        Store(0, Local1)
        Store (Acquire(FMT0, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            //Store Virtual Fan 0 status to local variable
            Store(CVF0, Local1)
            Release(FMT0)
        }
        // Return Virtual Fan 0 status.
        Return(Local1)
    }

    // This method is called when the temperature goes above _AC0.
    // Regardless of other FAN states, set to ACF0 since this is max cooling state: temp > _AC0
    Method(_ON, 0, Serialized)
    {
        Store (Acquire(FMT0, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 0 On.
            Store(1, CVF0)
            Release(FMT0)
        }
        FNCL()
    }

    // This method is called when the temperature goes below _AC0.
    // If FAN1 is on, use its value (AC1F): _AC0 > temp > _AC1
    // If FAN1 is off, use FAN2 value (AC2F): _AC0 > _AC1 > temp
    Method(_OFF, 0, Serialized)
    {
        Store (Acquire(FMT0, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 0 Off.
            Store(0, CVF0)
            Release(FMT0)
        }
        FNCL()
    }
  }

  // Associate Virtual Fan 0 Power Resource with the FAN0 Device.
  Device(FAN0)
  {
    Name(_HID, EISAID("PNP0C0B"))
    Name(_UID,0)
    Name(_PR0, Package(1){FN00})
  }

  // Fan 1 = Package Processor Fan.
  PowerResource(FN01,0,0)
  {
    Method(_STA,0,Serialized)
    {
        Store(0, Local1)
        Store (Acquire(FMT1, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            //Store Virtual Fan 1 status to local variable
            Store(CVF1, Local1)
            Release(FMT1)
        }
        // Return Virtual Fan 1 status.
        Return(Local1)
    }

    // This method is called when the temperature goes above _AC1.
    // If FAN0 is on, do nothing since we're already at AC0F: temp > _AC0 > _AC1
    // If FAN0 is off, use FAN1 value (AC1F): _AC0 > temp > _AC1
    Method(_ON,0,Serialized)
    {
        Store (Acquire(FMT1, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 1 On.
            Store(1, CVF1)
            Release(FMT1)
        }
        FNCL()
    }

    // This method is called when the temperature goes below _AC1.
    // If FAN2 is on, use its value (AC2F): _AC1 > temp > _AC2
    // If FAN2 is off, use FAN3 value (AC3F): _AC1 > _AC2 > temp
    Method(_OFF,0,Serialized)
    {
        Store (Acquire(FMT1, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 1 Off.
            Store(0, CVF1)
            Release(FMT1)
        }
        FNCL()
    }
  }

  // Associate Virtual Fan 1 Power Resource with the FAN1 Device.
  Device(FAN1)
  {
    Name(_HID, EISAID("PNP0C0B"))
    Name(_UID, 1)
    Name(_PR0, Package(1){FN01})
  }


  // Fan 2 = Package Processor Fan.
  PowerResource(FN02,0,0)
  {
    Method(_STA,0,Serialized)
    {
        Store(0, Local1)
        Store (Acquire(FMT2, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            //Store Virtual Fan 2 status to local variable
            Store(CVF2, Local1)
            Release(FMT2)
        }
        // Return Virtual Fan 2 status.
        Return(Local1)
    }

    // This method is called when the temperature goes above _AC2.
    // If FAN1 is on, do nothing since we're already at AC1F or greater: temp > _AC1 > _AC2
    // If FAN1 is off, use FAN2 value (AC2F): _AC1 > temp > _AC2
    Method(_ON,0,Serialized)
    {
        Store (Acquire(FMT2, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 2 On.
            Store(1, CVF2)
            Release(FMT2)
        }
        FNCL()
    }

    // This method is called when the temperature goes below _AC2.
    // If FAN3 is on, use its value (AC3F): _AC2 > temp > _AC3
    // If FAN3 is off, use FAN4 value (AC4F): _AC2 > _AC3 > temp
    Method(_OFF,0,Serialized)
    {
        Store (Acquire(FMT2, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 2 Off.
            Store(0, CVF2)
            Release(FMT2)
        }
        FNCL()
    }
  }

  // Associate Virtual Fan 2 Power Resource with the FAN0 Device.
  Device(FAN2)
  {
    Name(_HID, EISAID("PNP0C0B"))
    Name(_UID, 2)
    Name(_PR0, Package(1){FN02})
  }

  // Fan 3 = Package Processor Fan.
  PowerResource(FN03,0,0)
  {
    Method(_STA,0,Serialized)
    {
        Store(0, Local1)
        Store (Acquire(FMT3, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            //Store Virtual Fan 3 status to local variable
            Store(CVF3, Local1)
            Release(FMT3)
        }
        // Return Virtual Fan 3 status.
        Return(Local1)
    }

    // This method is called when the temperature goes above _AC3.
    // If FAN2 is on, do nothing since we're already at AC2F or greater: temp > _AC2 > _AC3
    // If FAN2 is off, use FAN3 value (AC3F): _AC2 > temp > _AC3
    Method(_ON,0,Serialized)
    {
        Store (Acquire(FMT3, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 3 On.
            Store(1, CVF3)
            Release(FMT3)
        }
        FNCL()
    }

    // This method is called when the temperature goes below _AC3.
    // If FAN4 is on, use its value (AC4F): _AC3 > temp > _AC4
    // If FAN4 is off, use FAN5 value (AC5F): _AC3 > _AC4 > temp
    Method(_OFF,0,Serialized)
    {
        Store (Acquire(FMT3, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 3 Off.
            Store(0, CVF3)
            Release(FMT3)
        }
        FNCL()
    }
  }

  // Associate Virtual Fan 3 Power Resource with the FAN3 Device.
  Device(FAN3)
  {
    Name(_HID, EISAID("PNP0C0B"))
    Name(_UID, 3)
    Name(_PR0, Package(1){FN03})
  }

  // Fan 4 = Package Processor Fan - Lowest Fan Speed
  PowerResource(FN04,0,0)
  {
    Method(_STA,0,Serialized)
    {
        Store(0, Local1)
        Store (Acquire(FMT4, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            //Store Virtual Fan 4 status to local variable
            Store(CVF4, Local1)
            Release(FMT4)
        }
        // Return Virtual Fan 4 status.
        Return(Local1)
    }

    // This method is called when the temperature goes above _AC4.
    // If FAN3 is on, do nothing since we're already at AC3F or greater: temp > _AC3 > _AC4
    // If FAN3 is off, use FAN4 value (AC4F): _AC3 > temp > _AC4
    Method(_ON,0,Serialized)
    {
        Store (Acquire(FMT4, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 4 On.
            Store(1, CVF4)
            Release(FMT4)
        }
        FNCL()
    }

    // This method is called when the temperature goes below _AC4.
    // FAN4 is the lowest FAN state defined, so we simply go to AC4F
    Method(_OFF,0,Serialized)
    {
        Store (Acquire(FMT4, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
        If (LEqual(Local0, Zero))  // check for Mutex acquired
        {
            // Set Virtual Fan 4 Off.
            Store(0, CVF4)
            Release(FMT4)
        }
        FNCL()
    }
  }

  // Associate Virtual Fan 4 Power Resource with the FAN4 Device.
  Device(FAN4)
  {
    Name(_HID, EISAID("PNP0C0B"))
    Name(_UID, 4)
    Name(_PR0, Package(1){FN04})
  }

  //
  // Fan Control Method
  //
  Method(FNCL, 0, NotSerialized)
  {
      ADBG ("Run FNCL ()")
      //If we get here, then we need to actually update the Fan Speed
      //First get the state of the Virtual Fans
      Store(0, Local0)
      Store(0, Local1)
      Store(0, Local2)
      Store(0, Local3)
      Store(0, Local4)
      Store (Acquire(FMT0, 1000), Local5)  // save Acquire result so we can check for Mutex acquired
      If (LEqual(Local5, Zero))  // check for Mutex acquired
      {
          Store(CVF0, Local0)
          Release(FMT0)
      }
      Store (Acquire(FMT1, 1000), Local5)  // save Acquire result so we can check for Mutex acquired
      If (LEqual(Local5, Zero))  // check for Mutex acquired
      {
          Store(CVF1, Local1)
          Release(FMT1)
      }
      Store (Acquire(FMT2, 1000), Local5)  // save Acquire result so we can check for Mutex acquired
      If (LEqual(Local5, Zero))  // check for Mutex acquired
      {
          Store(CVF2, Local2)
          Release(FMT2)
      }
      Store (Acquire(FMT3, 1000), Local5)  // save Acquire result so we can check for Mutex acquired
      If (LEqual(Local5, Zero))  // check for Mutex acquired
      {
          Store(CVF3, Local3)
          Release(FMT3)
      }
      Store (Acquire(FMT4, 1000), Local5)  // save Acquire result so we can check for Mutex acquired
      If (LEqual(Local5, Zero))  // check for Mutex acquired
      {
          Store(CVF4, Local4)
          Release(FMT4)
      }

      //Update the Global NVS Variables
      Store(Local0, \VFN0)
      Store(Local1, \VFN1)
      Store(Local2, \VFN2)
      Store(Local3, \VFN3)
      Store(Local4, \VFN4)

      If (\ECON)
      {
        //
        // Update fan speed
        //
        \_SB.PC00.LPCB.UPFS (ETMD, Local0, Local1)
      }Else
      {
        //Virtual Fan0 and Fan1 are On - Set Real Fan to Full Speed
        If(LAnd(LNotEqual(Local0, 0), LNotEqual(Local1, 0)))
        {
          ADBG("SIO FAN Set Full Speed")
          \_SB.PC00.LPCB.ITE8.STFN(0xFF)
        }
        Else
        {
          //Virtual Fan0 is Off, Virtual Fan1 is On - Set Real Fan to 75% Speed
          If(LAnd(LEqual(Local0, 0), LNotEqual(Local1, 0)))
          {
            ADBG("SIO FAN Set 75 percent Speed")
            \_SB.PC00.LPCB.ITE8.STFN(0xC0)
          }
          //Otherwise - Turn the Real Fan Off
          Else
          {
            ADBG("SIO FAN Set Low Speed")
            \_SB.PC00.LPCB.ITE8.STFN(0x50)
          }
        }
      }
  }

  // Thermal Zone 0 = Package Thermal Zone.
  // Package Thermal Zone is used for Active and Critical Policy Control
  // Package Thermal Zone returns the maximum temperature
  // of all components within the package
  ThermalZone(TZ00)
  {
    // Temporary variable for holding the current temperature reading
    Name(PTMP,3000)

    // Notifies ASL Code the current cooling mode.
    //  0 - Active cooling
    //  1 - Passive cooling
    Method(_SCP, 1, Serialized)
    {
        Store(Arg0,\CTYP)
    }

    // Return the temperature at which the OS performs Critical Shutdown
    Method(_CRT, 0, Serialized)
    {
        // Returns automatic thermal reporting temperature for CPU throttling if available and valid.
        If(CondRefOf(\_SB.ACRT))
        {
            If(LNotEqual(\_SB.ACRT,0))
            {
                Return(Add(2732, Multiply(\_SB.ACRT, 10)))
            }
        }

        Return(Add(2732, Multiply(\CRTT, 10)))
    }

    // Return the temperature(s) at which the OS initiates Active Cooling.
    Method(_AC0, 0, Serialized)
    {
      // Returns automatic thermal reporting temperature for CPU throttling if available and valid.
      If(CondRefOf(\_SB.AAC0))
      {
        If(LNotEqual(\_SB.AAC0,0))
        {
          Return(Add(2732, Multiply(\_SB.AAC0, 10)))
        }
      }
      Return(Add(2732, Multiply(\ACTT, 10)))
    }

    Method(_AC1, 0, Serialized)
    {
      Return(Add(2732, Multiply(\ACT1, 10)))
    }

    Method(_AC2, 0, Serialized)
    {
      Return(Subtract(Add(2732, Multiply(\ACT1, 10)), 50)) // subtract 5 degrees from _AC1
    }

    Method(_AC3, 0, Serialized)
    {
      Return(Subtract(Add(2732, Multiply(\ACT1, 10)), 100)) // subtract 10 degrees from _AC1
    }

    Method(_AC4, 0, Serialized)
    {
      Return(Subtract(Add(2732, Multiply(\ACT1, 10)), 150)) // subtract 15 degrees from _AC1
    }

    // Return the device(s) to turn on when _ACx is exceeded.
    Name(_AL0, Package(){FAN0})
    Name(_AL1, Package(){FAN1})
    Name(_AL2, Package(){FAN2})
    Name(_AL3, Package(){FAN3})
    Name(_AL4, Package(){FAN4})

    // Return the Package Temperature.
    //    Source : Max Platform temperature returned by EC
    Method(_TMP, 0, Serialized)
    {
        // Source : Max Platform temperature returned by EC
        // If EC enabled/available
        If(\ECON)
        {
          If (LNot (ETMD))    // If Legacy TM is disabled, return static value
          {
            Return (3000)
          }
          // Get the max platform temperature.
          // ADBG("GET ECTEMP")
          Return (\_SB.PC00.LPCB.MXTP ())
        } Else
        {
          ADBG("SIO TEMPERATURE")
          Store(\_SB.PC00.LPCB.ITE8.GETT(0x2B), Local0) // Reading temperature from PCH DTS
          //ADBG(Concatenate("Temperature Value:", ToHexString(Local0)))
          If(LLessEqual (Local0, 0x7F))
          {
            Add(2732, Multiply(Local0, 10), Local0)
          }Else
          {
            //ADBG("NEG VAL")
            Not (Local0, Local0)
            Add(Local0, 0x1, Local0)
            Subtract(2732, Multiply(Local0, 10), Local0)
          }

          //ADBG(Concatenate("Temperature Value K:", ToHexString(Local0)))
          //Store(Local0, PTMP)
          Return(Local0)
        }
        Return (3000)
    }

    // Return the Processor (s) used for Passive Cooling.
    Method (_PSL, 0, Serialized)
    {
        If (LEqual (\TCNT, 48))
        {
            //  CMP - Throttling controls 48 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38, \_SB.PR39, \_SB.PR40, \_SB.PR41, \_SB.PR42, \_SB.PR43, \_SB.PR44, \_SB.PR45, \_SB.PR46, \_SB.PR47})
        }

        If (LEqual (\TCNT, 47))
        {
            //  CMP - Throttling controls 47 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38, \_SB.PR39, \_SB.PR40, \_SB.PR41, \_SB.PR42, \_SB.PR43, \_SB.PR44, \_SB.PR45, \_SB.PR46})
        }

        If (LEqual (\TCNT, 46))
        {
            //  CMP - Throttling controls 46 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38, \_SB.PR39, \_SB.PR40, \_SB.PR41, \_SB.PR42, \_SB.PR43, \_SB.PR44, \_SB.PR45})
        }

        If (LEqual (\TCNT, 45))
        {
            //  CMP - Throttling controls 45 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38, \_SB.PR39, \_SB.PR40, \_SB.PR41, \_SB.PR42, \_SB.PR43, \_SB.PR44})
        }

        If (LEqual (\TCNT, 44))
        {
            //  CMP - Throttling controls 44 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38, \_SB.PR39, \_SB.PR40, \_SB.PR41, \_SB.PR42, \_SB.PR43})
        }

        If (LEqual (\TCNT, 43))
        {
            //  CMP - Throttling controls 43 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38, \_SB.PR39, \_SB.PR40, \_SB.PR41, \_SB.PR42})
        }

        If (LEqual (\TCNT, 42))
        {
            //  CMP - Throttling controls 42 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38, \_SB.PR39, \_SB.PR40, \_SB.PR41})
        }

        If (LEqual (\TCNT, 41))
        {
            //  CMP - Throttling controls 41 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38, \_SB.PR39, \_SB.PR40})
        }

        If (LEqual (\TCNT, 40))
        {
            //  CMP - Throttling controls 40 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38, \_SB.PR39})
        }

        If (LEqual (\TCNT, 39))
        {
            //  CMP - Throttling controls 39 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37, \_SB.PR38})
        }

        If (LEqual (\TCNT, 38))
        {
            //  CMP - Throttling controls 38 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36, \_SB.PR37})
        }

        If (LEqual (\TCNT, 37))
        {
            //  CMP - Throttling controls 37 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35, \_SB.PR36})
        }

        If (LEqual (\TCNT, 36))
        {
            //  CMP - Throttling controls 36 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34, \_SB.PR35})
        }

        If (LEqual (\TCNT, 35))
        {
            //  CMP - Throttling controls 35 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33, \_SB.PR34})
        }

        If (LEqual (\TCNT, 34))
        {
            //  CMP - Throttling controls 34 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32, \_SB.PR33})
        }

        If (LEqual (\TCNT, 33))
        {
            //  CMP - Throttling controls 33 logical CPUs.
            Return (Package () {\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31, \_SB.PR32})
        }

        If (LEqual (\TCNT, 32))
        {
            //  CMP - Throttling controls 32 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30, \_SB.PR31})
        }

        If(LEqual(\TCNT, 31))
        {
            //  CMP - Throttling controls 31 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29, \_SB.PR30})
        }

        If(LEqual(\TCNT, 30))
        {
            //  CMP - Throttling controls 30 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28, \_SB.PR29})
        }

        If(LEqual(\TCNT, 29))
        {
            //  CMP - Throttling controls 29 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27, \_SB.PR28})
        }

        If(LEqual(\TCNT, 28))
        {
            //  CMP - Throttling controls 28 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26, \_SB.PR27})
        }

        If(LEqual(\TCNT, 27))
        {
            //  CMP - Throttling controls 27 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25, \_SB.PR26})
        }

        If(LEqual(\TCNT, 26))
        {
            //  CMP - Throttling controls 26 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24, \_SB.PR25})
        }

        If(LEqual(\TCNT, 25))
        {
            //  CMP - Throttling controls 25 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23, \_SB.PR24})
        }

        If(LEqual(\TCNT, 24))
        {
            //  CMP - Throttling controls 24 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22, \_SB.PR23})
        }

        If(LEqual(\TCNT, 23))
        {
            //  CMP - Throttling controls 23 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21, \_SB.PR22})
        }

        If(LEqual(\TCNT, 22))
        {
            //  CMP - Throttling controls 22 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20, \_SB.PR21})
        }

        If(LEqual(\TCNT, 21))
        {
            //  CMP - Throttling controls 21 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19, \_SB.PR20})
        }

        If(LEqual(\TCNT, 20))
        {
            //  CMP - Throttling controls 20 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18, \_SB.PR19})
        }

        If(LEqual(\TCNT, 19))
        {
            //  CMP - Throttling controls 19 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17, \_SB.PR18})
        }

        If(LEqual(\TCNT, 18))
        {
            //  CMP - Throttling controls 18 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16, \_SB.PR17})
        }

        If(LEqual(\TCNT, 17))
        {
            //  CMP - Throttling controls 17 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15, \_SB.PR16})
        }
        If(LEqual(\TCNT, 16))
        {
            //  CMP - Throttling controls 16 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13,\_SB.PR14,\_SB.PR15})
        }

        If(LEqual(\TCNT, 14))
        {
            //  CMP - Throttling controls 14 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11,\_SB.PR12,\_SB.PR13})
        }

        If(LEqual(\TCNT, 12))
        {
            //  CMP - Throttling controls 12 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09,\_SB.PR10,\_SB.PR11})
        }

        If(LEqual(\TCNT, 10))
        {
            //  CMP - Throttling controls 10 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07,\_SB.PR08,\_SB.PR09})
        }

        If(LEqual(\TCNT, 8))
        {
            //  CMP - Throttling controls 8 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06,\_SB.PR07})
        }

        If(LEqual(\TCNT, 7))
        {
            //  CMP - Throttling controls 7 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05,\_SB.PR06})
        }


        If(LEqual(\TCNT, 6))
        {
            //  CMP - Throttling controls 6 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04,\_SB.PR05})
        }

        If(LEqual(\TCNT, 5))
        {
            //  CMP - Throttling controls 5 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03,\_SB.PR04})
        }

        If(LEqual(\TCNT, 4))
        {
            //  CMP - Throttling controls 4 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02,\_SB.PR03})
        }

        If(LEqual(\TCNT, 3))
        {
            //  CMP - Throttling controls 3 logical CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01,\_SB.PR02})
        }


        If(LEqual(\TCNT, 2))
        {
            //  CMP - Throttling controls 2 CPUs.
            Return(Package(){\_SB.PR00,\_SB.PR01})
        }

        Return(Package(){\_SB.PR00})
    }

    // Returns the temperature at which the OS initiates CPU throttling.
    Method(_PSV, 0, Serialized)
    {
        // Returns automatic thermal reporting temperature for CPU throttling if available and valid.
        If(CondRefOf(\_SB.APSV))
        {
            If(LNotEqual(\_SB.APSV,0))
            {
                Return(Add(2732, Multiply(\_SB.APSV, 10)))
            }
        }
        Return(Add(2732, Multiply(\PSVT, 10)))
    }

    // Returns TC1 value used in the passive cooling formula.
    Method(_TC1, 0, Serialized)
    {
        Return(\TC1V)
    }

    // Returns TC2 value used in the passive cooling formula.
    Method(_TC2, 0, Serialized)
    {
        Return(\TC2V)
    }

    // Returns the sampling period used in the passive cooling formula.
    Method(_TSP, 0, Serialized)
    {
        Return(\TSPV)
    }
  }
 }
}
