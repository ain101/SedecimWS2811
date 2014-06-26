Die zwei Files :

"stm32f4xx.h"
"system_stm32f4xx.c"

sind für ein System mit einem 8MHz externen Quarz Clock

der interne Systemclock wird auf 168MHz eingestellt


beide Files muessen in den Projekteordner unter :
 "cmsis_boot"


oder im CooCox-Verzeichnis unter :

CoIDE\repo\Components\500_CMSIS BOOT\src\cmsis_boot\

(dann werden sie jedesmal beim neuanlegen eines Projektes geladen)

