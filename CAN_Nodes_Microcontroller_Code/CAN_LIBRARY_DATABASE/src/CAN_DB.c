/* Generated by DBCC, see <https://github.com/howerj/dbcc> */
#include "CAN_DB.h"
#include <inttypes.h>
#include <assert.h>

#define UNUSED(X) ((void)(X))

static inline uint64_t reverse_byte_order(uint64_t x) {
	x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
	x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
	x = (x & 0x00FF00FF00FF00FF) << 8  | (x & 0xFF00FF00FF00FF00) >> 8;
	return x;
}

static inline int print_helper(int r, int print_return_value) {
	return ((r >= 0) && (print_return_value >= 0)) ? r + print_return_value : -1;
}

static int pack_can_0x064_Request_Heartbeat(can_obj_can_db_h_t *o, uint64_t *data) {
	assert(o);
	assert(data);
	register uint64_t x;
	register uint64_t i = 0;
	/* Sig_Req_Heartbeat: start-bit 0, length 64, endianess intel, scaling 1, offset 0 */
	x = ((uint64_t)(o->can_0x064_Request_Heartbeat.Sig_Req_Heartbeat)) & 0xffffffffffffffff;
	i |= x;
	*data = (i);
	o->can_0x064_Request_Heartbeat_tx = 1;
	return 8;
}

static int unpack_can_0x064_Request_Heartbeat(can_obj_can_db_h_t *o, uint64_t data, uint8_t dlc, dbcc_time_stamp_t time_stamp) {
	assert(o);
	assert(dlc <= 8);
	register uint64_t x;
	register uint64_t i = (data);
	if (dlc < 8)
		return -1;
	/* Sig_Req_Heartbeat: start-bit 0, length 64, endianess intel, scaling 1, offset 0 */
	x = i & 0xffffffffffffffff;
	o->can_0x064_Request_Heartbeat.Sig_Req_Heartbeat = x;
	o->can_0x064_Request_Heartbeat_rx = 1;
	o->can_0x064_Request_Heartbeat_time_stamp_rx = time_stamp;
	return 8;
}

int decode_can_0x064_Sig_Req_Heartbeat(const can_obj_can_db_h_t *o, uint64_t *out) {
	assert(o);
	assert(out);
	uint64_t rval = (uint64_t)(o->can_0x064_Request_Heartbeat.Sig_Req_Heartbeat);
	if (rval <= 255) {
		*out = rval;
		return 0;
	} else {
		*out = (uint64_t)0;
		return -1;
	}
}

int encode_can_0x064_Sig_Req_Heartbeat(can_obj_can_db_h_t *o, uint64_t in) {
	assert(o);
	o->can_0x064_Request_Heartbeat.Sig_Req_Heartbeat = 0;
	if (in > 255)
		return -1;
	o->can_0x064_Request_Heartbeat.Sig_Req_Heartbeat = in;
	return 0;
}

int print_can_0x064_Request_Heartbeat(const can_obj_can_db_h_t *o, FILE *output) {
	assert(o);
	assert(output);
	int r = 0;
	r = print_helper(r, fprintf(output, "Sig_Req_Heartbeat = (wire: %.0f)\n", (double)(o->can_0x064_Request_Heartbeat.Sig_Req_Heartbeat)));
	return r;
}

static int pack_can_0x065_Response_Heartbeat_SPCU(can_obj_can_db_h_t *o, uint64_t *data) {
	assert(o);
	assert(data);
	register uint64_t x;
	register uint64_t i = 0;
	/* Response_Heartbeat_sig: start-bit 0, length 8, endianess intel, scaling 1, offset 0 */
	x = ((uint8_t)(o->can_0x065_Response_Heartbeat_SPCU.Response_Heartbeat_sig)) & 0xff;
	i |= x;
	*data = (i);
	o->can_0x065_Response_Heartbeat_SPCU_tx = 1;
	return 8;
}

static int unpack_can_0x065_Response_Heartbeat_SPCU(can_obj_can_db_h_t *o, uint64_t data, uint8_t dlc, dbcc_time_stamp_t time_stamp) {
	assert(o);
	assert(dlc <= 8);
	register uint64_t x;
	register uint64_t i = (data);
	if (dlc < 8)
		return -1;
	/* Response_Heartbeat_sig: start-bit 0, length 8, endianess intel, scaling 1, offset 0 */
	x = i & 0xff;
	o->can_0x065_Response_Heartbeat_SPCU.Response_Heartbeat_sig = x;
	o->can_0x065_Response_Heartbeat_SPCU_rx = 1;
	o->can_0x065_Response_Heartbeat_SPCU_time_stamp_rx = time_stamp;
	return 8;
}

int decode_can_0x065_Response_Heartbeat_sig(const can_obj_can_db_h_t *o, uint8_t *out) {
	assert(o);
	assert(out);
	uint8_t rval = (uint8_t)(o->can_0x065_Response_Heartbeat_SPCU.Response_Heartbeat_sig);
	if (rval <= 1) {
		*out = rval;
		return 0;
	} else {
		*out = (uint8_t)0;
		return -1;
	}
}

int encode_can_0x065_Response_Heartbeat_sig(can_obj_can_db_h_t *o, uint8_t in) {
	assert(o);
	o->can_0x065_Response_Heartbeat_SPCU.Response_Heartbeat_sig = 0;
	if (in > 1)
		return -1;
	o->can_0x065_Response_Heartbeat_SPCU.Response_Heartbeat_sig = in;
	return 0;
}

int print_can_0x065_Response_Heartbeat_SPCU(const can_obj_can_db_h_t *o, FILE *output) {
	assert(o);
	assert(output);
	int r = 0;
	r = print_helper(r, fprintf(output, "Response_Heartbeat_sig = (wire: %.0f)\n", (double)(o->can_0x065_Response_Heartbeat_SPCU.Response_Heartbeat_sig)));
	return r;
}

static int pack_can_0x066_Response_Heartbeat_XXX(can_obj_can_db_h_t *o, uint64_t *data) {
	assert(o);
	assert(data);
	UNUSED(o);
	UNUSED(data);
	o->can_0x066_Response_Heartbeat_XXX_tx = 1;
	return 8;
}

static int unpack_can_0x066_Response_Heartbeat_XXX(can_obj_can_db_h_t *o, uint64_t data, uint8_t dlc, dbcc_time_stamp_t time_stamp) {
	assert(o);
	assert(dlc <= 8);
	UNUSED(o);
	UNUSED(data);
	if (dlc < 8)
		return -1;
	o->can_0x066_Response_Heartbeat_XXX_rx = 1;
	o->can_0x066_Response_Heartbeat_XXX_time_stamp_rx = time_stamp;
	return 8;
}

int print_can_0x066_Response_Heartbeat_XXX(const can_obj_can_db_h_t *o, FILE *output) {
	assert(o);
	assert(output);
	UNUSED(o);
	UNUSED(output);
	return 0;
}

static int pack_can_0x1f4_Error_SPCU(can_obj_can_db_h_t *o, uint64_t *data) {
	assert(o);
	assert(data);
	register uint64_t x;
	register uint64_t i = 0;
	/* Heartbeat: start-bit 0, length 1, endianess intel, scaling 1, offset 0 */
	x = ((uint8_t)(o->can_0x1f4_Error_SPCU.Heartbeat)) & 0x1;
	i |= x;
	/* Propulsion_Error: start-bit 1, length 1, endianess intel, scaling 1, offset 0 */
	x = ((uint8_t)(o->can_0x1f4_Error_SPCU.Propulsion_Error)) & 0x1;
	x <<= 1; 
	i |= x;
	/* Steering_Error: start-bit 2, length 1, endianess intel, scaling 1, offset 0 */
	x = ((uint8_t)(o->can_0x1f4_Error_SPCU.Steering_Error)) & 0x1;
	x <<= 2; 
	i |= x;
	*data = (i);
	o->can_0x1f4_Error_SPCU_tx = 1;
	return 1;
}

static int unpack_can_0x1f4_Error_SPCU(can_obj_can_db_h_t *o, uint64_t data, uint8_t dlc, dbcc_time_stamp_t time_stamp) {
	assert(o);
	assert(dlc <= 8);
	register uint64_t x;
	register uint64_t i = (data);
	if (dlc < 1)
		return -1;
	/* Heartbeat: start-bit 0, length 1, endianess intel, scaling 1, offset 0 */
	x = i & 0x1;
	o->can_0x1f4_Error_SPCU.Heartbeat = x;
	/* Propulsion_Error: start-bit 1, length 1, endianess intel, scaling 1, offset 0 */
	x = (i >> 1) & 0x1;
	o->can_0x1f4_Error_SPCU.Propulsion_Error = x;
	/* Steering_Error: start-bit 2, length 1, endianess intel, scaling 1, offset 0 */
	x = (i >> 2) & 0x1;
	o->can_0x1f4_Error_SPCU.Steering_Error = x;
	o->can_0x1f4_Error_SPCU_rx = 1;
	o->can_0x1f4_Error_SPCU_time_stamp_rx = time_stamp;
	return 1;
}

int decode_can_0x1f4_Heartbeat(const can_obj_can_db_h_t *o, uint8_t *out) {
	assert(o);
	assert(out);
	uint8_t rval = (uint8_t)(o->can_0x1f4_Error_SPCU.Heartbeat);
	*out = rval;
	return 0;
}

int encode_can_0x1f4_Heartbeat(can_obj_can_db_h_t *o, uint8_t in) {
	assert(o);
	o->can_0x1f4_Error_SPCU.Heartbeat = in;
	return 0;
}

int decode_can_0x1f4_Propulsion_Error(const can_obj_can_db_h_t *o, uint8_t *out) {
	assert(o);
	assert(out);
	uint8_t rval = (uint8_t)(o->can_0x1f4_Error_SPCU.Propulsion_Error);
	*out = rval;
	return 0;
}

int encode_can_0x1f4_Propulsion_Error(can_obj_can_db_h_t *o, uint8_t in) {
	assert(o);
	o->can_0x1f4_Error_SPCU.Propulsion_Error = in;
	return 0;
}

int decode_can_0x1f4_Steering_Error(const can_obj_can_db_h_t *o, uint8_t *out) {
	assert(o);
	assert(out);
	uint8_t rval = (uint8_t)(o->can_0x1f4_Error_SPCU.Steering_Error);
	*out = rval;
	return 0;
}

int encode_can_0x1f4_Steering_Error(can_obj_can_db_h_t *o, uint8_t in) {
	assert(o);
	o->can_0x1f4_Error_SPCU.Steering_Error = in;
	return 0;
}

int print_can_0x1f4_Error_SPCU(const can_obj_can_db_h_t *o, FILE *output) {
	assert(o);
	assert(output);
	int r = 0;
	r = print_helper(r, fprintf(output, "Heartbeat = (wire: %.0f)\n", (double)(o->can_0x1f4_Error_SPCU.Heartbeat)));
	r = print_helper(r, fprintf(output, "Propulsion_Error = (wire: %.0f)\n", (double)(o->can_0x1f4_Error_SPCU.Propulsion_Error)));
	r = print_helper(r, fprintf(output, "Steering_Error = (wire: %.0f)\n", (double)(o->can_0x1f4_Error_SPCU.Steering_Error)));
	return r;
}

static int pack_can_0x3e8_Set_SPCU(can_obj_can_db_h_t *o, uint64_t *data) {
	assert(o);
	assert(data);
	register uint64_t x;
	register uint64_t i = 0;
	/* Act_BreakVoltage: start-bit 0, length 13, endianess intel, scaling 1, offset 0 */
	x = ((uint16_t)(o->can_0x3e8_Set_SPCU.Act_BreakVoltage)) & 0x1fff;
	i |= x;
	/* Act_ThrottleVoltage: start-bit 33, length 13, endianess intel, scaling 1, offset 0 */
	x = ((uint16_t)(o->can_0x3e8_Set_SPCU.Act_ThrottleVoltage)) & 0x1fff;
	x <<= 33; 
	i |= x;
	/* Act_SteeringPosition: start-bit 17, length 8, endianess intel, scaling 1, offset 0 */
	x = ((uint8_t)(o->can_0x3e8_Set_SPCU.Act_SteeringPosition)) & 0xff;
	x <<= 17; 
	i |= x;
	/* Act_SteeringVelocity: start-bit 25, length 8, endianess intel, scaling 1, offset 0 */
	x = ((uint8_t)(o->can_0x3e8_Set_SPCU.Act_SteeringVelocity)) & 0xff;
	x <<= 25; 
	i |= x;
	/* Act_Reverse: start-bit 16, length 1, endianess intel, scaling 1, offset 0 */
	x = ((uint8_t)(o->can_0x3e8_Set_SPCU.Act_Reverse)) & 0x1;
	x <<= 16; 
	i |= x;
	*data = (i);
	o->can_0x3e8_Set_SPCU_tx = 1;
	return 8;
}

static int unpack_can_0x3e8_Set_SPCU(can_obj_can_db_h_t *o, uint64_t data, uint8_t dlc, dbcc_time_stamp_t time_stamp) {
	assert(o);
	assert(dlc <= 8);
	register uint64_t x;
	register uint64_t i = (data);
	if (dlc < 8)
		return -1;
	/* Act_BreakVoltage: start-bit 0, length 13, endianess intel, scaling 1, offset 0 */
	x = i & 0x1fff;
	o->can_0x3e8_Set_SPCU.Act_BreakVoltage = x;
	/* Act_ThrottleVoltage: start-bit 33, length 13, endianess intel, scaling 1, offset 0 */
	x = (i >> 33) & 0x1fff;
	o->can_0x3e8_Set_SPCU.Act_ThrottleVoltage = x;
	/* Act_SteeringPosition: start-bit 17, length 8, endianess intel, scaling 1, offset 0 */
	x = (i >> 17) & 0xff;
	o->can_0x3e8_Set_SPCU.Act_SteeringPosition = x;
	/* Act_SteeringVelocity: start-bit 25, length 8, endianess intel, scaling 1, offset 0 */
	x = (i >> 25) & 0xff;
	o->can_0x3e8_Set_SPCU.Act_SteeringVelocity = x;
	/* Act_Reverse: start-bit 16, length 1, endianess intel, scaling 1, offset 0 */
	x = (i >> 16) & 0x1;
	o->can_0x3e8_Set_SPCU.Act_Reverse = x;
	o->can_0x3e8_Set_SPCU_rx = 1;
	o->can_0x3e8_Set_SPCU_time_stamp_rx = time_stamp;
	return 8;
}

int decode_can_0x3e8_Act_BreakVoltage(const can_obj_can_db_h_t *o, uint16_t *out) {
	assert(o);
	assert(out);
	uint16_t rval = (uint16_t)(o->can_0x3e8_Set_SPCU.Act_BreakVoltage);
	if ((rval >= 850) && (rval <= 5100)) {
		*out = rval;
		return 0;
	} else {
		*out = (uint16_t)0;
		return -1;
	}
}

int encode_can_0x3e8_Act_BreakVoltage(can_obj_can_db_h_t *o, uint16_t in) {
	assert(o);
	o->can_0x3e8_Set_SPCU.Act_BreakVoltage = 0;
	if (in < 850)
		return -1;
	if (in > 5100)
		return -1;
	o->can_0x3e8_Set_SPCU.Act_BreakVoltage = in;
	return 0;
}

int decode_can_0x3e8_Act_ThrottleVoltage(const can_obj_can_db_h_t *o, uint16_t *out) {
	assert(o);
	assert(out);
	uint16_t rval = (uint16_t)(o->can_0x3e8_Set_SPCU.Act_ThrottleVoltage);
	if ((rval >= 850) && (rval <= 5100)) {
		*out = rval;
		return 0;
	} else {
		*out = (uint16_t)0;
		return -1;
	}
}

int encode_can_0x3e8_Act_ThrottleVoltage(can_obj_can_db_h_t *o, uint16_t in) {
	assert(o);
	o->can_0x3e8_Set_SPCU.Act_ThrottleVoltage = 0;
	if (in < 850)
		return -1;
	if (in > 5100)
		return -1;
	o->can_0x3e8_Set_SPCU.Act_ThrottleVoltage = in;
	return 0;
}

int decode_can_0x3e8_Act_SteeringPosition(const can_obj_can_db_h_t *o, int8_t *out) {
	assert(o);
	assert(out);
	int8_t rval = (int8_t)(o->can_0x3e8_Set_SPCU.Act_SteeringPosition);
	*out = rval;
	return 0;
}

int encode_can_0x3e8_Act_SteeringPosition(can_obj_can_db_h_t *o, int8_t in) {
	assert(o);
	o->can_0x3e8_Set_SPCU.Act_SteeringPosition = in;
	return 0;
}

int decode_can_0x3e8_Act_SteeringVelocity(const can_obj_can_db_h_t *o, uint8_t *out) {
	assert(o);
	assert(out);
	uint8_t rval = (uint8_t)(o->can_0x3e8_Set_SPCU.Act_SteeringVelocity);
	*out = rval;
	return 0;
}

int encode_can_0x3e8_Act_SteeringVelocity(can_obj_can_db_h_t *o, uint8_t in) {
	assert(o);
	o->can_0x3e8_Set_SPCU.Act_SteeringVelocity = in;
	return 0;
}

int decode_can_0x3e8_Act_Reverse(const can_obj_can_db_h_t *o, uint8_t *out) {
	assert(o);
	assert(out);
	uint8_t rval = (uint8_t)(o->can_0x3e8_Set_SPCU.Act_Reverse);
	*out = rval;
	return 0;
}

int encode_can_0x3e8_Act_Reverse(can_obj_can_db_h_t *o, uint8_t in) {
	assert(o);
	o->can_0x3e8_Set_SPCU.Act_Reverse = in;
	return 0;
}

int print_can_0x3e8_Set_SPCU(const can_obj_can_db_h_t *o, FILE *output) {
	assert(o);
	assert(output);
	int r = 0;
	r = print_helper(r, fprintf(output, "Act_BreakVoltage = (wire: %.0f)\n", (double)(o->can_0x3e8_Set_SPCU.Act_BreakVoltage)));
	r = print_helper(r, fprintf(output, "Act_ThrottleVoltage = (wire: %.0f)\n", (double)(o->can_0x3e8_Set_SPCU.Act_ThrottleVoltage)));
	r = print_helper(r, fprintf(output, "Act_SteeringPosition = (wire: %.0f)\n", (double)(o->can_0x3e8_Set_SPCU.Act_SteeringPosition)));
	r = print_helper(r, fprintf(output, "Act_SteeringVelocity = (wire: %.0f)\n", (double)(o->can_0x3e8_Set_SPCU.Act_SteeringVelocity)));
	r = print_helper(r, fprintf(output, "Act_Reverse = (wire: %.0f)\n", (double)(o->can_0x3e8_Set_SPCU.Act_Reverse)));
	return r;
}

static int pack_can_0x5dc_Get_Speed_Sensor(can_obj_can_db_h_t *o, uint64_t *data) {
	assert(o);
	assert(data);
	register uint64_t x;
	register uint64_t i = 0;
	/* Get_Velocity: start-bit 0, length 8, endianess intel, scaling 100, offset 0 */
	x = ((uint8_t)(o->can_0x5dc_Get_Speed_Sensor.Get_Velocity)) & 0xff;
	i |= x;
	*data = (i);
	o->can_0x5dc_Get_Speed_Sensor_tx = 1;
	return 8;
}

static int unpack_can_0x5dc_Get_Speed_Sensor(can_obj_can_db_h_t *o, uint64_t data, uint8_t dlc, dbcc_time_stamp_t time_stamp) {
	assert(o);
	assert(dlc <= 8);
	register uint64_t x;
	register uint64_t i = (data);
	if (dlc < 8)
		return -1;
	/* Get_Velocity: start-bit 0, length 8, endianess intel, scaling 100, offset 0 */
	x = i & 0xff;
	o->can_0x5dc_Get_Speed_Sensor.Get_Velocity = x;
	o->can_0x5dc_Get_Speed_Sensor_rx = 1;
	o->can_0x5dc_Get_Speed_Sensor_time_stamp_rx = time_stamp;
	return 8;
}

int decode_can_0x5dc_Get_Velocity(const can_obj_can_db_h_t *o, uint8_t *out) {
	assert(o);
	assert(out);
	//uint8_t rval = (uint8_t)(o->can_0x5dc_Get_Speed_Sensor.Get_Velocity);
	//rval *= 100;
	//*out = rval;
	*out = o->can_0x5dc_Get_Speed_Sensor.Get_Velocity;
	return 0;
}

int encode_can_0x5dc_Get_Velocity(can_obj_can_db_h_t *o, uint8_t in) {
	assert(o);
	//in *= 0.01;
	o->can_0x5dc_Get_Speed_Sensor.Get_Velocity = in;
	return 0;
}

int print_can_0x5dc_Get_Speed_Sensor(const can_obj_can_db_h_t *o, FILE *output) {
	assert(o);
	assert(output);
	int r = 0;
	r = print_helper(r, fprintf(output, "Get_Velocity = (wire: %.0f)\n", (double)(o->can_0x5dc_Get_Speed_Sensor.Get_Velocity)));
	return r;
}

static int pack_can_0x7d0_Get_SPCU(can_obj_can_db_h_t *o, uint64_t *data) {
	assert(o);
	assert(data);
	register uint64_t x;
	register uint64_t i = 0;
	/* Get_SteeringAngle: start-bit 0, length 9, endianess intel, scaling 1, offset 0 */
	x = ((uint16_t)(o->can_0x7d0_Get_SPCU.Get_SteeringAngle)) & 0x1ff;
	i |= x;
	/* Get_ReverseMode: start-bit 9, length 1, endianess intel, scaling 1, offset 0 */
	x = ((uint8_t)(o->can_0x7d0_Get_SPCU.Get_ReverseMode)) & 0x1;
	x <<= 9; 
	i |= x;
	*data = (i);
	o->can_0x7d0_Get_SPCU_tx = 1;
	return 8;
}

static int unpack_can_0x7d0_Get_SPCU(can_obj_can_db_h_t *o, uint64_t data, uint8_t dlc, dbcc_time_stamp_t time_stamp) {
	assert(o);
	assert(dlc <= 8);
	register uint64_t x;
	register uint64_t i = (data);
	if (dlc < 8)
		return -1;
	/* Get_SteeringAngle: start-bit 0, length 9, endianess intel, scaling 1, offset 0 */
	x = i & 0x1ff;
	x = (x & 0x100) ? (x | 0xfe00) : x; 
	o->can_0x7d0_Get_SPCU.Get_SteeringAngle = x;
	/* Get_ReverseMode: start-bit 9, length 1, endianess intel, scaling 1, offset 0 */
	x = (i >> 9) & 0x1;
	o->can_0x7d0_Get_SPCU.Get_ReverseMode = x;
	o->can_0x7d0_Get_SPCU_rx = 1;
	o->can_0x7d0_Get_SPCU_time_stamp_rx = time_stamp;
	return 8;
}

int decode_can_0x7d0_Get_SteeringAngle(const can_obj_can_db_h_t *o, int16_t *out) {
	assert(o);
	assert(out);
	int16_t rval = (int16_t)(o->can_0x7d0_Get_SPCU.Get_SteeringAngle);
	if ((rval >= -180) && (rval <= 180)) {
		*out = rval;
		return 0;
	} else {
		*out = (int16_t)0;
		return -1;
	}
}

int encode_can_0x7d0_Get_SteeringAngle(can_obj_can_db_h_t *o, int16_t in) {
	assert(o);
	o->can_0x7d0_Get_SPCU.Get_SteeringAngle = 0;
	if (in < -180)
		return -1;
	if (in > 180)
		return -1;
	o->can_0x7d0_Get_SPCU.Get_SteeringAngle = in;
	return 0;
}

int decode_can_0x7d0_Get_ReverseMode(const can_obj_can_db_h_t *o, uint8_t *out) {
	assert(o);
	assert(out);
	uint8_t rval = (uint8_t)(o->can_0x7d0_Get_SPCU.Get_ReverseMode);
	*out = rval;
	return 0;
}

int encode_can_0x7d0_Get_ReverseMode(can_obj_can_db_h_t *o, uint8_t in) {
	assert(o);
	o->can_0x7d0_Get_SPCU.Get_ReverseMode = in;
	return 0;
}

int print_can_0x7d0_Get_SPCU(const can_obj_can_db_h_t *o, FILE *output) {
	assert(o);
	assert(output);
	int r = 0;
	r = print_helper(r, fprintf(output, "Get_SteeringAngle = (wire: %.0f)\n", (double)(o->can_0x7d0_Get_SPCU.Get_SteeringAngle)));
	r = print_helper(r, fprintf(output, "Get_ReverseMode = (wire: %.0f)\n", (double)(o->can_0x7d0_Get_SPCU.Get_ReverseMode)));
	return r;
}

int unpack_message(can_obj_can_db_h_t *o, const unsigned long id, uint64_t data, uint8_t dlc, dbcc_time_stamp_t time_stamp) {
	assert(o);
	assert(id < (1ul << 29)); /* 29-bit CAN ID is largest possible */
	assert(dlc <= 8);         /* Maximum of 8 bytes in a CAN packet */
	switch (id) {
	case 0x064: return unpack_can_0x064_Request_Heartbeat(o, data, dlc, time_stamp);
	case 0x065: return unpack_can_0x065_Response_Heartbeat_SPCU(o, data, dlc, time_stamp);
	case 0x066: return unpack_can_0x066_Response_Heartbeat_XXX(o, data, dlc, time_stamp);
	case 0x1f4: return unpack_can_0x1f4_Error_SPCU(o, data, dlc, time_stamp);
	case 0x3e8: return unpack_can_0x3e8_Set_SPCU(o, data, dlc, time_stamp);
	case 0x5dc: return unpack_can_0x5dc_Get_Speed_Sensor(o, data, dlc, time_stamp);
	case 0x7d0: return unpack_can_0x7d0_Get_SPCU(o, data, dlc, time_stamp);
	default: break; 
	}
	return -1; 
}

int pack_message(can_obj_can_db_h_t *o, const unsigned long id, uint64_t *data) {
	assert(o);
	assert(id < (1ul << 29)); /* 29-bit CAN ID is largest possible */
	switch (id) {
	case 0x064: return pack_can_0x064_Request_Heartbeat(o, data);
	case 0x065: return pack_can_0x065_Response_Heartbeat_SPCU(o, data);
	case 0x066: return pack_can_0x066_Response_Heartbeat_XXX(o, data);
	case 0x1f4: return pack_can_0x1f4_Error_SPCU(o, data);
	case 0x3e8: return pack_can_0x3e8_Set_SPCU(o, data);
	case 0x5dc: return pack_can_0x5dc_Get_Speed_Sensor(o, data);
	case 0x7d0: return pack_can_0x7d0_Get_SPCU(o, data);
	default: break; 
	}
	return -1; 
}

int print_message(const can_obj_can_db_h_t *o, const unsigned long id, FILE *output) {
	assert(o);
	assert(id < (1ul << 29)); /* 29-bit CAN ID is largest possible */
	assert(output);
	switch (id) {
	case 0x064: return print_can_0x064_Request_Heartbeat(o, output);
	case 0x065: return print_can_0x065_Response_Heartbeat_SPCU(o, output);
	case 0x066: return print_can_0x066_Response_Heartbeat_XXX(o, output);
	case 0x1f4: return print_can_0x1f4_Error_SPCU(o, output);
	case 0x3e8: return print_can_0x3e8_Set_SPCU(o, output);
	case 0x5dc: return print_can_0x5dc_Get_Speed_Sensor(o, output);
	case 0x7d0: return print_can_0x7d0_Get_SPCU(o, output);
	default: break; 
	}
	return -1; 
}
