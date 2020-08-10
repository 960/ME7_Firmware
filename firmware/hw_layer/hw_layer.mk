HW_LAYER_EGT = $(PROJECT_DIR)/hw_layer/serial_over_usb/usbcfg.c \
	$(PROJECT_DIR)/hw_layer/serial_over_usb/usbconsole.c

HW_LAYER_INC=	$(PROJECT_DIR)/hw_layer $(PROJECT_DIR)/hw_layer/adc \
	$(PROJECT_DIR)/hw_layer/digital_input \
	$(PROJECT_DIR)/hw_layer/digital_input/trigger

HW_INC = hw_layer/$(CPU_HWLAYER) \
	$(PROJECT_DIR)/hw_layer/ports


HW_LAYER_EMS = $(HW_LAYER_EGT) \
	$(PROJECT_DIR)/hw_layer/mc33816_data.c

HW_LAYER_EMS_CPP = $(HW_LAYER_EGT_CPP) \
	$(PROJECT_DIR)/hw_layer/pin_repository.cpp \
	$(PROJECT_DIR)/hw_layer/microsecond_timer.cpp \
	$(PROJECT_DIR)/hw_layer/digital_input/digital_input.cpp \
	$(PROJECT_DIR)/hw_layer/digital_input/digital_input_icu.cpp \
	$(PROJECT_DIR)/hw_layer/digital_input/digital_input_exti.cpp \
	$(PROJECT_DIR)/hw_layer/digital_input/trigger/trigger_input.cpp \
	$(PROJECT_DIR)/hw_layer/digital_input/trigger/trigger_input_icu.cpp \
	$(PROJECT_DIR)/hw_layer/digital_input/trigger/trigger_input_exti.cpp \
	$(PROJECT_DIR)/hw_layer/hardware.cpp \
	$(PROJECT_DIR)/hw_layer/smart_gpio.cpp \
	$(PROJECT_DIR)/hw_layer/adc/adc_inputs.cpp \
	$(PROJECT_DIR)/hw_layer/adc/adc_subscription.cpp \
	$(PROJECT_DIR)/hw_layer/vehicle_speed.cpp \
	$(PROJECT_DIR)/hw_layer/stepper.cpp \
	$(PROJECT_DIR)/hw_layer/stepper_dual_hbridge.cpp \
	$(PROJECT_DIR)/hw_layer/io_pins.cpp \
	$(PROJECT_DIR)/hw_layer/rtc_helper.cpp \
	$(PROJECT_DIR)/hw_layer/backup_ram.cpp 

#
# '-include' is a magic kind of 'include' which would survive if file to be included is not found
#	
-include $(PROJECT_DIR)/hw_layer/$(CPU_HWLAYER)/hw_ports.mk
	