#ifndef SERIAL_OBJECT_H
#define SERIAL_OBJECT_H

class SerialObject {
protected:
	static unsigned long int instance_counter;
	unsigned long int id; 
public:
	SerialObject();
	~SerialObject();
	static unsigned long int get_instance_counter();
	unsigned long int get_id();	
};

#endif