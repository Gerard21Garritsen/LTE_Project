This is an IoT project example using MQTT and HTTP server by web side, and by hardware side I use a microcontroller STM32F411CE which connects with a SIMCOM LTE 4G module to connects
to the Nodejs server which had a MQTT server up.

STM32F411CE gets GPS data from SIMCOM LTE module and send it through MQTT protocol, also STM32F4 reads some analog data from sensors and send it to MQTT server.

By MQTT sever side, I get data from hardware and store it in a Data Base to after that, I use a graphical tool to show the data in a bar graphics.
