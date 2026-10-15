# STM32 USB Audio & MIDI DSP Controller

Sistema de tiempo real *bare-metal* sobre STM32L475 (ARM Cortex-M4) que emula dos tipos de dispositivos de sonido de clase USB: una tarjeta de audio externa con procesamiento DSP en vivo (efecto Delay) y un instrumento/sintetizador MIDI controlado por sensores inerciales.

Este proyecto aplica la arquitectura **Primer Plano / Segundo Plano (Foreground/Background)**, gestionando procesos asíncronos y transferencias con restricciones temporales críticas sin la asistencia de un Sistema Operativo en Tiempo Real (RTOS).

---

## Estructura del Repositorio

El repositorio se compone de dos proyectos independientes para STM32CubeIDE correspondientes a las diferentes clases USB configuradas:

```
├── audystreaming/                              # Proyecto 1: Audio Streaming USB y Efecto DSP
│   └── Core/
│       ├── Inc/
│       │   ├── hd44780.h              # Driver de pantalla LCD
│       │   ├── main.h
│       │   ├── stm32l4xx_hal_conf.h
│       │   ├── stm32l4xx_it.h
│       │   └── usbd_audio_if.h
│       └── Src/
│           ├── hd44780.c              # Rutinas de control LCD
│           ├── main.c                 # Rutinas ADC, LCD y reconstrucción DAC con TIM7
│           ├── stm32l4xx_hal_msp.c
│           ├── stm32l4xx_it.c         # Manejadores de interrupción
│           ├── syscalls.c
│           ├── sysmem.c
│           ├── system_stm32l4xx.c
│           └── usbd_audio_if.c        # Tarea de volcado USB a buffer circular
└── intysint/                         # Proyecto 2: Instrumento y Sintetizador USB MIDI
    └── Core/
        ├── Inc/
        │   ├── LSM6DSL.h              # Interface del sensor de acelerómetro/giroscopio
        │   ├── main.h
        │   ├── stm32l4xx_hal_conf.h
        │   ├── stm32l4xx_it.h
        │   ├── usb_device.h
        │   ├── usbd_conf.h
        │   ├── usbd_desc.h
        │   └── usbd_midi.h            # Interfaz de la clase MIDI de terceros
        └── Src/
            ├── LSM6DSL.c              # Lectura del sensor de impacto
            ├── main.c                 # Lógica de síntesis PWM y comandos Note ON/OFF
            ├── stm32l4xx_hal_msp.c
            ├── stm32l4xx_it.c
            ├── syscalls.c
            ├── sysmem.c
            ├── system_stm32l4xx.c
            ├── usb_device.c           # Inicialización parcheada y registro del Stack
            ├── usbd_conf.c
            ├── usbd_desc.c
            └── usbd_midi.c            # Drivers del dispositivo de clase MIDI
```

---

## Modos de Operación y Periféricos

### 1\. Altavoz USB PCM con Efecto Delay (`audystreaming`)

El microcontrolador se enumera como un dispositivo de salida estándar de la clase de **Audio Streaming**, recibiendo flujos PCM síncronos desde el host (16-bits a 24 kHz) y reproduciéndolos analógicamente.

* **Reconstrucción de Audio (DAC \+ Timer 7):** El PC envía ráfagas asíncronas de muestras a través del bus USB, las cuales se almacenan en un buffer circular. La conversión al dominio analógico (pin `PA5`) está orquestada estrictamente por el desbordamiento del **TIM7** a 24 kHz. La recarga del Timer fue ligeramente sub-ajustada (`ARR=3300`) a 24.24kHz como medida empírica para paliar la deriva de relojes asíncronos entre PC y placa, evitando saturación del buffer (buffer overflow) y 'clicks' acústicos.  
* **Procesamiento de Audio en Vivo (DSP):** La señal de entrada se promedia matemáticamente con un vector histórico dentro del buffer circular para producir un efecto de eco (*Delay*).  
* **Control en Segundo Plano (ADC):** Un bucle de baja prioridad captura el voltaje de la red resistiva de la botonera en la LCD Shield mediante el **ADC1**, ajustando en tiempo real las variables de ganancia y profundidad del retardo.

### 2\. Instrumento y Sintetizador MIDI (`intysint`)

Debido a la ausencia de soporte nativo en el configurador HAL para **MidiStreaming**, la pila de ST fue emulada inicialmente como Custom HID y los descriptores parcheados e inyectados manualmente en `usb_device.c` y `usbd_conf.c` para acoplar la librería `usbd_midi.c`.

* **Controlador Inercial (MIDI IN):** El sensor MEMS **LSM6DSL** es muestreado vía `I2C2`. Los movimientos físicos en los 3 ejes se procesan, escalando la magnitud bruta en impactos que desencadenan transferencias USB de 4 bytes hacia el PC (Eventos de canal 10: caja acústica, `Note ON/OFF`, velocidad dinámica).  
* **Síntesis Monofónica de Sonido (MIDI OUT):** El sistema extrae asíncronamente los índices musicales y calcula matemáticamente las frecuencias físicas dictadas por la escala temperada occidental (440 2^(n-69)/12). La nota musical generada moldea dinámicamente el periodo (frecuencia de pitch) y el *Duty Cycle* (volumen por velocidad) del generador de hardware **TIM2 (PWM)** en el pin `PA5`.

---

## Requisitos de Hardware y Compilación

* **Hardware:** Kit B-L475E-IOT01A (o cualquier núcleo STM32 equivalente adaptando el árbol de pines) \+ DFRobot LCD Keypad Shield.  
* Altavoz de bajo amperaje conectado entre `PA5` y masa (`GND`).  
* Software Host: Para la simulación MIDI se requiere VMPK (Virtual MIDI Piano Keyboard) u otro DAW que permita asignar E/S MIDI.  
* Compilar utilizando STM32CubeIDE (versión GCC ARM). Asegurarse de que el compilador tenga acceso en el Include Path a las librerías proporcionadas.
