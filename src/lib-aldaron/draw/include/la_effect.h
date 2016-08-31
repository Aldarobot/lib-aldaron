/** Structure for lights. */
typedef struct {
	jl_vec3_t position;
	jl_vec3_t color;
	float power;
}la_light_t;

void la_effect_light(jl_vo_t* vo, la_light_t* lights, uint8_t light_count,
	jl_vec3_t material_brightness);
