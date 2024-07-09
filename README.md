# Sistema-de-Control-de-Robot-Movil
Este proyecto abarca el diseño, la electrónica y el control tanto por WiFi como por USB de un robot móvil.

Se configuró y programó la placa STM32F103C8T6 y se flasheó el módulo ESP8266. A continuación, se detallan más aspectos del desarrollo:
Diseño y Desarrollo del Software de Control: Se desarrolló una aplicación en Qt que permite interactuar con el robot vía WiFi utilizando una ESP8266 que se comunica mediante UDP con la STM32F103C8T6. Adicionalmente, esta comunicación también puede realizarse y depurarse vía USB a través del puerto serial (USB) de la STM32F103C8T6.

Sistema de Control del Robot: El robot cuenta con un sistema de control PID que le permite seguir una pista guiándose por las paredes. Este sistema de control envía toda la información al software de Qt para visualizar gráficas y guardar los datos de los sensores en un archivo.

Desarrollo Electrónico: Se diseñó y desarrolló toda la electrónica necesaria para el funcionamiento del robot, incluyendo la configuración y programación de la placa STM32F103C8T6 y el flasheo del módulo ESP8266.

Para más información, se adjunta un archivo PDF con la explicación detallada del proyecto.

