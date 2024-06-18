//
// Automatically generated by GenNvs ver 2.4.6
// Please DO NOT modify !!!
//

/** @file

  Copyright (c) 2024, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

  //
  // Define BIOS Guard NVS Area operation region.
  //


  OperationRegion(BGNV,SystemMemory,0xFFFF0000,0xAA55)
  Field(BGNV,AnyAcc,Lock,Preserve)
  {  Offset(0),      BGMA, 64, // Offset(0),     BIOS Guard Memory Address for Tool Interface
  Offset(8),      BGMS, 8,  // Offset(8),     BIOS Guard Memory Size for Tool Interface
  Offset(9),      BGIA, 8,  // Offset(9),     BIOS Guard SW SMI value for Tool Interface
  }
