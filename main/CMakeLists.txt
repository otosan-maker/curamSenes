set(COMPONENT_SRCS "main.c" "blink.c" "ui.c" "wifi.c" "mqtt.c" "curamSenes.c" "speech.c" "blueScan.c")
set(COMPONENT_ADD_INCLUDEDIRS "." "./includes" )
set(COMPONENT_REQUIRES "nvs_flash" "esp-aws-iot" "esp-cryptoauthlib" "core2forAWS" "json_parser")
register_component()

target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
