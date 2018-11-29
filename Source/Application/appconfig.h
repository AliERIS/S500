#ifndef _APPCONFIG_H_
#define _APPCONFIG_H_

/* LCD interface settings */
#define LCD_XRESOLUTION 240
#define LCD_YRESOLUTION 240


/* Application pins configuration */

/* LCD interface */
#define LCD_RESET       GPIO45                                                                      //LCDIF Reset
#define LCD_RESET_MODE  GPIO45_MODE_LSRSTB
#define LCD_CE          GPIO46                                                                      //LCDIF Chip Select
#define LCD_CE_MODE     GPIO46_MODE_LSCE_B0
#define LCD_SCK         GPIO47                                                                      //LCDIF Clock
#define LCD_SCK_MODE    GPIO47_MODE_LSCK0
#define LCD_SDA         GPIO48                                                                      //LCDIF Data IO line 0
#define LCD_SDA_MODE    GPIO48_MODE_LSDA0

#endif /* _APPCONFIG_H_ */
