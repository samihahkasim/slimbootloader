/** @file
  GpioSiLib implementation for Alder Lake platform

  Copyright (c) 2024, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PciLib.h>
#include <Library/GpioSiLib.h>
#include <Library/BootloaderCommonLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchInfoLib.h>
#include <Register/PchRegsGpioVer2.h>
#include <Register/PchRegsGpioVer4.h>
#include <Register/PchRegsPmcVer2.h>
#include <Register/PchRegsPmcVer4.h>
#include <Library/PchInfoLib.h>
#include <Library/PchSbiAccessLib.h>
#include <Register/PmcRegs.h>
#include <Register/PchRegsPcr.h>
#include "GpioPinsVer2Lp.h"
#include "GpioPinsVer4S.h"
#include <Register/PchRegsGpio.h>
#include <Library/ConfigDataLib.h>
#include <Include/PcrDefine.h>

#define GPIO_PAD_DEF(Group,Pad)                (UINT32)(((Group) << 16) + (Pad))
#define GPIO_GROUP_DEF(GroupIndex,ChipsetId)   ((GroupIndex) | ((ChipsetId) << 8))
#define GPIO_GET_GROUP_INDEX(Group)            ((Group) & 0x1F)
#define GPIO_GET_GROUP_FROM_PAD(GpioPad)       (((GpioPad) & 0x0F1F0000) >> 16)
#define GPIO_GET_GROUP_INDEX_FROM_PAD(GpioPad) GPIO_GET_GROUP_INDEX (GPIO_GET_GROUP_FROM_PAD(GpioPad))
#define GPIO_GET_PAD_NUMBER(GpioPad)           ((GpioPad) & 0x1FF)
#define GPIO_GET_CHIPSET_ID(GpioPad)           (((GpioPad) >> 24) & 0xF)

GLOBAL_REMOVE_IF_UNREFERENCED GPIO_GROUP_INFO mPchLpGpioGroupInfo[] = {
  {PID_GPIOCOM0, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_GPI_GPE_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_SMI_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_SMI_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_NMI_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_NMI_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_B_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_B_PAD_MAX},
  {PID_GPIOCOM0, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_T_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_T_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_T_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_T_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_T_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_T_GPI_GPE_EN, NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_T_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_T_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_T_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_T_PAD_MAX},
  {PID_GPIOCOM0, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_A_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_A_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_A_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_A_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_A_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_A_GPI_GPE_EN, NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_A_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_A_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_A_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_A_PAD_MAX},
  {PID_GPIOCOM5, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_R_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_R_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_R_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_R_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_R_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_R_GPI_GPE_EN, NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_R_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_R_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_R_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_R_PAD_MAX},
  {PID_GPIOCOM5, R_GPIO_VER2_PCH_LP_GPIO_PCR_SPI_PAD_OWN,    R_GPIO_VER2_PCH_LP_GPIO_PCR_SPI_HOSTSW_OWN,    NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,                NO_REGISTER_FOR_PROPERTY,               NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_SPI_PADCFGLOCK,    R_GPIO_VER2_PCH_LP_GPIO_PCR_SPI_PADCFGLOCKTX,     R_GPIO_VER2_PCH_LP_GPIO_PCR_SPI_PADCFG_OFFSET,    GPIO_VER2_PCH_LP_GPIO_SPI_PAD_MAX},
  {PID_GPIOCOM2, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPD_PAD_OWN,    R_GPIO_VER2_PCH_LP_GPIO_PCR_GPD_HOSTSW_OWN,    R_GPIO_VER2_PCH_LP_GPIO_PCR_GPD_GPI_IS,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPD_GPI_IE,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPD_GPI_GPE_STS,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPD_GPI_GPE_EN,   NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_GPD_PADCFGLOCK,    R_GPIO_VER2_PCH_LP_GPIO_PCR_GPD_PADCFGLOCKTX,     R_GPIO_VER2_PCH_LP_GPIO_PCR_GPD_PADCFG_OFFSET,    GPIO_VER2_PCH_LP_GPIO_GPD_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_S_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_S_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_S_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_S_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_S_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_S_GPI_GPE_EN, NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_S_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_S_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_S_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_S_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_H_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_H_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_H_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_H_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_H_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_H_GPI_GPE_EN, NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,            R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_H_PADCFGLOCK, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_H_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_H_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_H_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_GPI_GPE_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_SMI_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_SMI_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_NMI_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_NMI_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_D_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_D_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_U_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_U_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_U_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_U_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_U_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_U_GPI_GPE_EN, NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_U_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_U_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_U_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_U_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_GPI_GPE_EN, NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_VGPIO_PAD_MAX},
  {PID_GPIOCOM4, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_GPI_GPE_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_SMI_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_SMI_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_NMI_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_NMI_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_C_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_C_PAD_MAX},
  {PID_GPIOCOM4, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_F_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_F_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_F_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_F_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_F_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_F_GPI_GPE_EN, NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_F_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_F_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_F_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_F_PAD_MAX},
  {PID_GPIOCOM4, R_GPIO_VER2_PCH_LP_GPIO_PCR_HVCMOS_PAD_OWN, R_GPIO_VER2_PCH_LP_GPIO_PCR_HVCMOS_HOSTSW_OWN, NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,                NO_REGISTER_FOR_PROPERTY,               NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_HVCMOS_PADCFGLOCK, R_GPIO_VER2_PCH_LP_GPIO_PCR_HVCMOS_PADCFGLOCKTX,  R_GPIO_VER2_PCH_LP_GPIO_PCR_HVCMOS_PADCFG_OFFSET, GPIO_VER2_PCH_LP_GPIO_HVCMOS_PAD_MAX},
  {PID_GPIOCOM4, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_PAD_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_HOSTSW_OWN,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_GPI_IS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_GPI_IE, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_GPI_GPE_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_GPI_GPE_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_SMI_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_SMI_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_NMI_STS, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_NMI_EN, R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_PADCFGLOCK,  R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_PADCFGLOCKTX,   R_GPIO_VER2_PCH_LP_GPIO_PCR_GPP_E_PADCFG_OFFSET,  GPIO_VER2_PCH_LP_GPIO_GPP_E_PAD_MAX},
  {PID_GPIOCOM4, R_GPIO_VER2_PCH_LP_GPIO_PCR_JTAG_PAD_OWN,   R_GPIO_VER2_PCH_LP_GPIO_PCR_JTAG_HOSTSW_OWN,   NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,                NO_REGISTER_FOR_PROPERTY,               NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_JTAG_PADCFGLOCK,   R_GPIO_VER2_PCH_LP_GPIO_PCR_JTAG_PADCFGLOCKTX,    R_GPIO_VER2_PCH_LP_GPIO_PCR_JTAG_PADCFG_OFFSET,   GPIO_VER2_PCH_LP_GPIO_JTAG_PAD_MAX},
  {PID_GPIOCOM3, R_GPIO_VER2_PCH_LP_GPIO_PCR_CPU_PAD_OWN,    R_GPIO_VER2_PCH_LP_GPIO_PCR_CPU_HOSTSW_OWN,    NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,                NO_REGISTER_FOR_PROPERTY,               NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_CPU_PADCFGLOCK,    R_GPIO_VER2_PCH_LP_GPIO_PCR_CPU_PADCFGLOCKTX,     R_GPIO_VER2_PCH_LP_GPIO_PCR_CPU_PADCFG_OFFSET,    GPIO_VER2_PCH_LP_GPIO_CPU_PAD_MAX},
  {PID_GPIOCOM3, R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_3_PAD_OWN,R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_3_HOSTSW_OWN,NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,                NO_REGISTER_FOR_PROPERTY,               NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           NO_REGISTER_FOR_PROPERTY,            NO_REGISTER_FOR_PROPERTY,           R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_3_PADCFGLOCK,R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_3_PADCFGLOCKTX, R_GPIO_VER2_PCH_LP_GPIO_PCR_VGPIO_3_PADCFG_OFFSET,GPIO_VER2_PCH_LP_GPIO_VGPIO_3_PAD_MAX}
};

GLOBAL_REMOVE_IF_UNREFERENCED GPIO_GROUP_INFO mSocMGpioGroupInfo[] = {
  {PID_GPIOCOM0, R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_PAD_OWN,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_HOSTSW_OWN,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_GPI_IS,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_GPI_IE,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_GPI_GPE_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_GPI_GPE_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_I_SMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_I_SMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_NMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_NMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_PADCFGLOCK,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_PADCFGLOCKTX,    R_GPIO_VER6_SOC_M_GPIO_PCR_CPU_PADCFG_OFFSET,    GPIO_VER6_SOC_M_CPU_PAD_MAX},
  {PID_GPIOCOM0, R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_I_SMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_I_SMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_V_PADCFG_OFFSET,  GPIO_VER6_SOC_M_GPP_V_PAD_MAX},
  {PID_GPIOCOM0, R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_I_SMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_I_SMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_C_PADCFG_OFFSET,  GPIO_VER6_SOC_M_GPP_C_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_SMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_SMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_A_PADCFG_OFFSET,  GPIO_VER6_SOC_M_GPP_A_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_SMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_SMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_E_PADCFG_OFFSET,  GPIO_VER6_SOC_M_GPP_E_PAD_MAX},
  {PID_GPIOCOM3, R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_SMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_SMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_H_PADCFG_OFFSET,  GPIO_VER6_SOC_M_GPP_H_PAD_MAX},
  {PID_GPIOCOM3, R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_SMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_SMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_F_PADCFG_OFFSET,  GPIO_VER6_SOC_M_GPP_F_PAD_MAX},
  {PID_GPIOCOM3, R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_PAD_OWN,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_HOSTSW_OWN,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_GPI_IS,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_GPI_IE,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_GPI_GPE_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_GPI_GPE_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_SMI_STS,      R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_SMI_EN,      R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_NMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_NMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_PADCFGLOCK,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_PADCFGLOCKTX,    R_GPIO_VER6_SOC_M_GPIO_PCR_SPI_PADCFG_OFFSET,    GPIO_VER6_SOC_M_SPI_SYS_PAD_MAX},
  {PID_GPIOCOM3, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_SMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_SMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_PADCFG_OFFSET,  GPIO_VER6_SOC_M_USB_THC_PAD_MAX},
  {PID_GPIOCOM4, R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_SMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_SMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_S_PADCFG_OFFSET,  GPIO_VER6_SOC_M_GPP_S_PAD_MAX},
  {PID_GPIOCOM4, R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_PAD_OWN,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_HOSTSW_OWN,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_GPI_IS,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_GPI_IE,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_GPI_GPE_STS,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_GPI_GPE_EN,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_SMI_STS,     R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_SMI_EN,     R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_NMI_STS,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_NMI_EN,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_PADCFGLOCK,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_PADCFGLOCKTX,   R_GPIO_VER6_SOC_M_GPIO_PCR_JTAG_PADCFG_OFFSET,   GPIO_VER6_SOC_M_JTAG_PAD_MAX},
  {PID_GPIOCOM5, R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_SMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_SMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_B_PADCFG_OFFSET,  GPIO_VER6_SOC_M_GPP_B_PAD_MAX},
  {PID_GPIOCOM5, R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_PAD_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_HOSTSW_OWN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_GPI_IS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_GPI_IE,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_GPI_GPE_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_GPI_GPE_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_SMI_STS,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_SMI_EN,    R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_NMI_STS,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_NMI_EN,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_PADCFGLOCK,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_GPP_D_PADCFG_OFFSET,  GPIO_VER6_SOC_M_GPP_D_PAD_MAX},
  {PID_GPIOCOM5, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_PAD_OWN, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_HOSTSW_OWN, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_GPI_IS, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_GPI_IE, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_GPI_GPE_STS, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_GPI_GPE_EN, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_SMI_STS,   R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_SMI_EN,   R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_NMI_STS, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_NMI_EN, R_GPIO_VER6_SOC_M_GPIO_PCR_VPGIO5_PADCFGLOCK, R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO_PADCFGLOCKTX,  R_GPIO_VER6_SOC_M_GPIO_PCR_VGPIO5_PADCFG_OFFSET, GPIO_VER6_SOC_M_VGPIO_PAD_MAX}
};

GLOBAL_REMOVE_IF_UNREFERENCED GPIO_GROUP_INFO mSocSGpioGroupInfo[] = {
  {PID_GPIOCOM0, R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_PAD_OWN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_HOSTSW_OWN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_GPI_IS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_GPI_IE,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_GPI_GPE_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_GPI_GPE_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_SMI_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_SMI_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_NMI_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_NMI_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_PADCFGLOCK,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_PADCFGLOCKTX,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_A_PADCFG_OFFSET,   GPIO_VER6_SOC_S_GPP_A_PAD_MAX},
  {PID_GPIOCOM0, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_PAD_OWN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_HOSTSW_OWN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_GPI_IS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_GPI_IE, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_GPI_GPE_STS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_GPI_GPE_EN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_SMI_STS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_SMI_EN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_NMI_STS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_NMI_EN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_PADCFGLOCK, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_PADCFGLOCKTX, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_0_PADCFG_OFFSET, GPIO_VER6_SOC_S_VGPIO_0_PAD_MAX },
  {PID_GPIOCOM0, R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_PAD_OWN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_HOSTSW_OWN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_GPI_IS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_GPI_IE,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_GPI_GPE_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_GPI_GPE_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_SMI_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_SMI_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_NMI_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_NMI_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_PADCFGLOCK,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_PADCFGLOCKTX,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_C_PADCFG_OFFSET,   GPIO_VER6_SOC_S_GPP_C_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_PAD_OWN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_HOSTSW_OWN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_GPI_IS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_GPI_IE,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_GPI_GPE_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_GPI_GPE_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_SMI_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_SMI_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_NMI_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_NMI_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_PADCFGLOCK,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_PADCFGLOCKTX,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_B_PADCFG_OFFSET,   GPIO_VER6_SOC_S_GPP_B_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_PAD_OWN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_HOSTSW_OWN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_GPI_IS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_GPI_IE, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_GPI_GPE_STS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_GPI_GPE_EN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_SMI_STS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_SMI_EN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_NMI_STS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_NMI_EN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_PADCFGLOCK, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_PADCFGLOCKTX, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_3_PADCFG_OFFSET, GPIO_VER6_SOC_S_VGPIO_3_PAD_MAX},
  {PID_GPIOCOM1, R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_PAD_OWN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_HOSTSW_OWN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_GPI_IS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_GPI_IE,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_GPI_GPE_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_GPI_GPE_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_SMI_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_SMI_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_NMI_STS,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_NMI_EN,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_PADCFGLOCK,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_PADCFGLOCKTX,   R_GPIO_VER6_SOC_S_GPIO_PCR_GPP_D_PADCFG_OFFSET,   GPIO_VER6_SOC_S_GPP_D_PAD_MAX},
  {PID_GPIOCOM3, R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_PAD_OWN,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_HOSTSW_OWN,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_GPI_IS,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_GPI_IE,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_GPI_GPE_STS,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_GPI_GPE_EN,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_SMI_STS,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_SMI_EN,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_NMI_STS,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_NMI_EN,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_PADCFGLOCK,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_PADCFGLOCKTX,    R_GPIO_VER6_SOC_S_GPIO_PCR_JTAG_PADCFG_OFFSET,    GPIO_VER6_SOC_S_JTAG_PAD_MAX},
  {PID_GPIOCOM3, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_PAD_OWN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_HOSTSW_OWN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_GPI_IS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_GPI_IE, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_GPI_GPE_STS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_GPI_GPE_EN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_SMI_STS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_SMI_EN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_NMI_STS, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_NMI_EN, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_PADCFGLOCK, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_PADCFGLOCKTX, R_GPIO_VER6_SOC_S_GPIO_PCR_VGPIO_4_PADCFG_OFFSET, GPIO_VER6_SOC_S_VGPIO_4_PAD_MAX}
};

GLOBAL_REMOVE_IF_UNREFERENCED GPIO_GROUP_TO_GPE_MAPPING mSocSGpioGroupToGpeMapping[] = {
  {GPIO_VER6_SOC_S_GROUP_GPP_A,   0, V_MTL_SOC_S_PMC_PWRM_GPIO_CFG_GPP_A,     V_GPIO_VER6_SOC_S_GPIO_PCR_MISCCFG_GPE0_GPP_A},
  {GPIO_VER6_SOC_S_GROUP_GPP_C,   0, V_MTL_SOC_S_PMC_PWRM_GPIO_CFG_GPP_C,     V_GPIO_VER6_SOC_S_GPIO_PCR_MISCCFG_GPE0_GPP_C},
  {GPIO_VER6_SOC_S_GROUP_GPP_B,   0, V_MTL_SOC_S_PMC_PWRM_GPIO_CFG_GPP_B,     V_GPIO_VER6_SOC_S_GPIO_PCR_MISCCFG_GPE0_GPP_B},
  {GPIO_VER6_SOC_S_GROUP_VGPIO_3, 0, V_MTL_SOC_S_PMC_PWRM_GPIO_CFG_VGPIO_3,   V_GPIO_VER6_SOC_S_GPIO_PCR_MISCCFG_GPE0_VGPIO_3},
  {GPIO_VER6_SOC_S_GROUP_VGPIO_4, 0, V_MTL_SOC_S_PMC_PWRM_GPIO_CFG_VGPIO_4,   V_GPIO_VER6_SOC_S_GPIO_PCR_MISCCFG_GPE0_VGPIO_4},
  {GPIO_VER6_SOC_S_GROUP_VGPIO_0, 0, V_MTL_SOC_S_PMC_PWRM_GPIO_CFG_VGPIO_0,   V_GPIO_VER6_SOC_S_GPIO_PCR_MISCCFG_GPE0_VGPIO_0},
  {GPIO_VER6_SOC_S_GROUP_GPP_D,   0, V_MTL_SOC_S_PMC_PWRM_GPIO_CFG_GPP_D,     V_GPIO_VER6_SOC_S_GPIO_PCR_MISCCFG_GPE0_GPP_D},
  {GPIO_VER6_SOC_S_GROUP_JTAG,    0, V_MTL_SOC_S_PMC_PWRM_GPIO_CFG_JTAG,      V_GPIO_VER6_SOC_S_GPIO_PCR_MISCCFG_GPE0_JTAG}
};

GLOBAL_REMOVE_IF_UNREFERENCED PCH_SBI_PID mGpioComSbiIds []=
{
  PID_GPIOCOM0, PID_GPIOCOM1, PID_GPIOCOM2, PID_GPIOCOM3, PID_GPIOCOM4, PID_GPIOCOM5
};

GLOBAL_REMOVE_IF_UNREFERENCED PCH_SBI_PID mSocMGpioComSbiIds[] =
{
  PID_GPIOCOM0, PID_GPIOCOM1, PID_GPIOCOM3, PID_GPIOCOM4, PID_GPIOCOM5
};

GLOBAL_REMOVE_IF_UNREFERENCED GPIO_GROUP_TO_GPE_MAPPING mPchLpGpioGroupToGpeMapping[] = {
  {GPIO_VER2_LP_GROUP_GPP_B,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_B,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_B},
  {GPIO_VER2_LP_GROUP_GPP_T,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_T,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_T},
  {GPIO_VER2_LP_GROUP_GPP_A,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_A,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_A},
  {GPIO_VER2_LP_GROUP_GPP_R,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_R,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_R},
  {GPIO_VER2_LP_GROUP_GPD,    0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPD,    V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPD},
  {GPIO_VER2_LP_GROUP_GPP_S,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_S,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_S},
  {GPIO_VER2_LP_GROUP_GPP_H,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_H,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_H},
  {GPIO_VER2_LP_GROUP_GPP_D,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_D,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_D},
  {GPIO_VER2_LP_GROUP_GPP_U,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_U,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_U},
  {GPIO_VER2_LP_GROUP_VGPIO,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_VGPIO,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_VGPIO},
  {GPIO_VER2_LP_GROUP_GPP_C,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_C,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_C},
  {GPIO_VER2_LP_GROUP_GPP_F,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_F,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_F},
  {GPIO_VER2_LP_GROUP_GPP_E,  0, V_TGL_PCH_LP_PMC_PWRM_GPIO_CFG_GPP_E,  V_GPIO_VER2_PCH_LP_GPIO_PCR_MISCCFG_GPE0_GPP_E}
};

GLOBAL_REMOVE_IF_UNREFERENCED GPIO_GROUP_TO_GPE_MAPPING mSocMGpioGroupToGpeMapping[] = {
  {GPIO_VER6_SOC_M_GROUP_CPU,     0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_CPU,     V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_CPU},
  {GPIO_VER6_SOC_M_GROUP_GPP_V,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_GPP_V,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_GPP_V},
  {GPIO_VER6_SOC_M_GROUP_GPP_C,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_GPP_C,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_GPP_C},
  {GPIO_VER6_SOC_M_GROUP_GPP_A,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_GPP_A,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_GPP_A},
  {GPIO_VER6_SOC_M_GROUP_GPP_E,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_GPP_E,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_GPP_E},
  {GPIO_VER6_SOC_M_GROUP_GPP_H,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_GPP_H,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_GPP_H},
  {GPIO_VER6_SOC_M_GROUP_GPP_F,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_GPP_F,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_GPP_F},
  {GPIO_VER6_SOC_M_GROUP_SPI_SYS, 0, V_MTL_SOC_M_PMC_PWRM_GPIO_SPI_SYS,     V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_SPI_SYS},
  {GPIO_VER6_SOC_M_GROUP_USB_THC, 0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_USB,     V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_USB},
  {GPIO_VER6_SOC_M_GROUP_GPP_S,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_GPP_S,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_GPP_S},
  {GPIO_VER6_SOC_M_GROUP_JTAG,    0, V_MTL_SOC_M_PMC_PWRM_GPIO_JTAG,        V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_JTAG},
  {GPIO_VER6_SOC_M_GROUP_GPP_B,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_GPP_B,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_GPP_B},
  {GPIO_VER6_SOC_M_GROUP_GPP_D,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_GPP_D,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_GPP_D},
  {GPIO_VER6_SOC_M_GROUP_VGPIO,   0, V_MTL_SOC_M_PMC_PWRM_GPIO_CFG_VGPIO,   V_GPIO_VER6_SOC_M_GPIO_PCR_MISCCFG_GPE0_VGPIO}
};

//
// GPIO_PAD Fileds
//
typedef union {
  struct {
    UINT32    PadNum      :16;
    UINT32    GrpIdx      :8;
    UINT32    ChipsetId   :4;
    UINT32    Rsvd        :4;
  } PadField;
  UINT32      Pad;
} PAD_INFO;

//
// GPIO_CFG_DATA DW1 fields
//
typedef struct {
  UINT32    Rsvd1       :16;
  UINT32    PadNum      :8;
  UINT32    GrpIdx      :5;
  UINT32    Rsvd2       :3;
} GPIO_CFG_DATA_DW1;

/**
  This function gets Group to GPE0 configuration

  @param[out] GpeDw0Value       GPIO Group to GPE_DW0 assignment
  @param[out] GpeDw1Value       GPIO Group to GPE_DW1 assignment
  @param[out] GpeDw2Value       GPIO Group to GPE_DW2 assignment
**/
VOID
EFIAPI
PmcGetGpioGpe (
  OUT UINT32    *GpeDw0Value,
  OUT UINT32    *GpeDw1Value,
  OUT UINT32    *GpeDw2Value
  )
{
  UINT32 Data32;

  Data32 = MmioRead32 (PCH_PWRM_BASE_ADDRESS + R_PMC_PWRM_GPIO_CFG);

  *GpeDw0Value = ((Data32 & B_PMC_PWRM_GPIO_CFG_GPE0_DW0) >> N_PMC_PWRM_GPIO_CFG_GPE0_DW0);
  *GpeDw1Value = ((Data32 & B_PMC_PWRM_GPIO_CFG_GPE0_DW1) >> N_PMC_PWRM_GPIO_CFG_GPE0_DW1);
  *GpeDw2Value = ((Data32 & B_PMC_PWRM_GPIO_CFG_GPE0_DW2) >> N_PMC_PWRM_GPIO_CFG_GPE0_DW2);
}

/**
  Return opcode supported for writing to GPIO lock unlock register

  @retval UINT8   Lock Opcode
**/
UINT8
EFIAPI
GpioGetLockOpcode (
  VOID
  )
{
  // if (IsPchS ()) {
  //   return PrivateControlWrite;
  // } else if (IsPchP ()) {
    // return GpioLockUnlock;
    return GpioLockUnlock;
  // } else {
  //   ASSERT (FALSE);
  //   return 0;
  // }
}

/**
  This internal procedure will check if group is within DeepSleepWell.

  @param[in]  Group               GPIO Group

  @retval GroupWell               TRUE:  This is DSW Group
                                  FALSE: This is not DSW Group
**/
BOOLEAN
EFIAPI
GpioIsDswGroup (
  IN  GPIO_GROUP         Group
  )
{
  // if ((Group == GPIO_VER4_S_GROUP_GPD) || (Group == GPIO_VER2_LP_GROUP_GPD)) {
  //   return TRUE;
  // } else {
  //   return FALSE;
  // }
  return FALSE;
}

/**
  This procedure will retrieve address and length of GPIO info table

  @param[out]  GpioGroupInfoTableLength   Length of GPIO group table

  @retval Pointer to GPIO group table

**/
CONST GPIO_GROUP_INFO*
EFIAPI
GpioGetGroupInfoTable (
  OUT UINT32              *GpioGroupInfoTableLength
  )
{
  if (MtlIsSocM ()) {
    *GpioGroupInfoTableLength = ARRAY_SIZE (mSocMGpioGroupInfo);
    return mSocMGpioGroupInfo;
  } else {
    *GpioGroupInfoTableLength = ARRAY_SIZE (mSocSGpioGroupInfo);
    return mSocSGpioGroupInfo;
  }
}

/**
  Get GPIO Chipset ID specific to PCH generation and series
**/
UINT32
EFIAPI
GpioGetThisChipsetId (
  VOID
  )
{
  if (MtlIsSocM ()) {
    return GPIO_VER6_SOC_M_CHIPSET_ID;
  } else if (MtlIsSocS ()) {
    return GPIO_VER6_SOC_S_CHIPSET_ID;
  } else {
    return 0;
  }
}

/**
  Get information for GPIO Group required to program GPIO and PMC for desired 1-Tier GPE mapping

  @param[out] GpioGroupToGpeMapping        Table with GPIO Group to GPE mapping
  @param[out] GpioGroupToGpeMappingLength  GPIO Group to GPE mapping table length
**/
VOID
EFIAPI
GpioGetGroupToGpeMapping (
  OUT GPIO_GROUP_TO_GPE_MAPPING  **GpioGroupToGpeMapping,
  OUT UINT32                     *GpioGroupToGpeMappingLength
  )
{
  if (MtlIsSocM ()) {
    *GpioGroupToGpeMapping = mSocMGpioGroupToGpeMapping;
    *GpioGroupToGpeMappingLength = ARRAY_SIZE (mSocMGpioGroupToGpeMapping);
  } else {
    *GpioGroupToGpeMapping = mSocSGpioGroupToGpeMapping;
    *GpioGroupToGpeMappingLength = ARRAY_SIZE (mSocSGpioGroupToGpeMapping);
  }
}

/**
  This procedure will perform special handling of GPP_A_12.

  @param[in]  None

  @retval None
**/
VOID
GpioA12SpecialHandling (
  VOID
  )
{
  return;
}

/**
  This function provides GPIO Community PortIDs

  @param[out] NativePinsTable                Table with GPIO COMMx SBI PortIDs

  @retval      Number of communities
**/
UINT32
EFIAPI
GpioGetComSbiPortIds (
  OUT PCH_SBI_PID    **GpioComSbiIds
  )
{
  // *GpioComSbiIds = mGpioComSbiIds;
  // return ARRAY_SIZE (mGpioComSbiIds);
  *GpioComSbiIds = mSocMGpioComSbiIds;
  return ARRAY_SIZE (mSocMGpioComSbiIds);
}


/**
  Get PCH PCR Address for this Register

  @param[in] Pid                        Port ID of the SBI message
  @param[in] Offset                     Offset of the Cfg Regsiter

  @retval UINT32                        Pch Pcr Address to access this register
**/
UINT32
EFIAPI
GetPchPcrAddress (
  IN     GPIO_PCH_SBI_PID               Pid,
  IN     UINT32                         Offset
  )
{
  return PCH_PCR_ADDRESS (Pid, Offset);
}

/**
  Get TypeC SBU Gpio Pad table

  @param[out] TableLength       Length of the TypeC SBU Gpio Pad table

  @retval Pointer to TypeC SBU Gpio Pad table
**/
GPIO_PAD*
EFIAPI
GpioGetTypeCSbuGpioPad (
  IN OUT   UINT32      *TableLength
)
{
  *TableLength = 0;
  return NULL;
}

/**
  Get GPIO PCR Pad Cfg Offset

  @retval PAD CFG Offset
**/
UINT8
EFIAPI
GpioGetPcrPadCfgOffset(
  VOID
  )
{
  return S_GPIO_PCR_PADCFG;
}


/**
  This procedure will return GpioPad from Group and PadNumber

  @param[in] Group              GPIO group
  @param[in] PadNumber          GPIO PadNumber

  @retval GpioPad               GpioPad
**/
GPIO_PAD
EFIAPI
GpioGetGpioPadFromGroupAndPadNumber (
  IN GPIO_GROUP      Group,
  IN UINT32          PadNumber
  )
{
  return GPIO_PAD_DEF (Group,PadNumber);
}


/**
  This procedure will return GpioPad from GroupIndex and PadNumber

  @param[in] GroupIndex         GPIO GroupIndex
  @param[in] PadNumber          GPIO PadNumber

  @retval GpioPad               GpioPad
**/
GPIO_PAD
EFIAPI
GpioGetGpioPadFromGroupIndexAndPadNumber (
  IN UINT32          GroupIndex,
  IN UINT32          PadNumber
  )
{
  GPIO_GROUP Group;

  Group = GPIO_GROUP_DEF (GroupIndex, GpioGetThisChipsetId ());
  return GPIO_PAD_DEF (Group, PadNumber);
}


/**
  This procedure will get group from group index (0 based)

  @param[in] GroupIndex        Group Index

  @retval GpioGroup            Gpio Group
**/
GPIO_GROUP
EFIAPI
GpioGetGroupFromGroupIndex (
  IN UINT32        GroupIndex
  )
{
  return GPIO_GROUP_DEF (GroupIndex, GpioGetThisChipsetId ());
}


/**
  This procedure will get group index (0 based) from group

  @param[in] GpioGroup        Gpio Group

  @retval Value               Group Index
**/
UINT32
EFIAPI
GpioGetGroupIndexFromGroup (
  IN GPIO_GROUP        GpioGroup
  )
{
  return (UINT32) GPIO_GET_GROUP_INDEX (GpioGroup);
}



/**
  This procedure will get group number

  @param[in] GpioPad          Gpio Pad

  @retval Value               Group number
**/
GPIO_GROUP
EFIAPI
GpioGetGroupFromGpioPad (
  IN GPIO_PAD         GpioPad
  )
{
  return GPIO_GET_GROUP_FROM_PAD (GpioPad);
}


/**
  This procedure will get group index (0 based)

  @param[in] GpioPad          Gpio Pad

  @retval Value               Group Index
**/
UINT32
EFIAPI
GpioGetGroupIndexFromGpioPad (
  IN GPIO_PAD        GpioPad
  )
{
  return (UINT32) GPIO_GET_GROUP_INDEX_FROM_PAD (GpioPad);
}


/**
  This procedure will get pad number (0 based) from Gpio Pad

  @param[in] GpioPad          Gpio Pad

  @retval Value               Pad Number
**/
UINT32
EFIAPI
GpioGetPadNumberFromGpioPad (
  IN GPIO_PAD        GpioPad
  )
{
  return (UINT32) GPIO_GET_PAD_NUMBER (GpioPad);
}

/**
  This procedure will get Chipset ID from Gpio Pad

  @param[in] GpioPad          Gpio Pad

  @retval Value               PCH Chipset ID
**/
UINT32
EFIAPI
GpioGetChipsetIdFromGpioPad (
  IN GPIO_PAD        GpioPad
  )
{
  return (UINT32) GPIO_GET_CHIPSET_ID (GpioPad);
}

/**
  This procedure will get Gpio Pad from Cfg Dword

  @param[in]  GpioItem         Pointer to the Gpio Cfg Data Item
  @param[out] GpioPad          Gpio Pad
**/
VOID
EFIAPI
GpioGetGpioPadFromCfgDw (
  IN  UINT32        *GpioItem,
  OUT GPIO_PAD      *GpioPad
  )
{
  GPIO_CFG_DATA_DW1    *Dw1;
  PAD_INFO              PadInfo;

  Dw1 = (GPIO_CFG_DATA_DW1 *) (&GpioItem[1]);

  PadInfo.PadField.PadNum    = (UINT16) Dw1->PadNum;
  PadInfo.PadField.GrpIdx    = (UINT8)  Dw1->GrpIdx;
  PadInfo.PadField.ChipsetId = GpioGetThisChipsetId ();
  *GpioPad = PadInfo.Pad;

  //
  // Remove PadInfo data from DW1
  //
  Dw1->PadNum = 0;
  Dw1->GrpIdx = 0;
}
