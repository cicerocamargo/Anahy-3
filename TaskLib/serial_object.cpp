#include "serial_object.h"

unsigned long int SerialObject::instance_counter = 0;

SerialObject::SerialObject() {
	id = instance_counter++;
}

SerialObject::~SerialObject() { }

unsigned long int SerialObject::get_instance_counter() {
	return instance_counter;
}

unsigned long int SerialObject::get_id() {
	return id;
}