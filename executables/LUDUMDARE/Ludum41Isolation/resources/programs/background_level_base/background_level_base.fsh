out vec4 output_color; // "output_color" replaces "gl_FragColor" because glBindFragDataLocation(...) has been called

uniform sampler2D background_clear;
uniform sampler2D background_hidden;
uniform sampler2D noise;

uniform float fill_ratio;
uniform float blendzone_ratio;
uniform vec4  canvas_box;

in vec2 vs_position;
in vec2 vs_texcoord;
in vec4 vs_color;

uniform float decal_speed;
uniform float decal_ratio;

uniform float game_time;

void main()
{



	vec2 pos = 2.0 * canvas_box.zw * ((vs_position * 0.5) + vec2(0.5, 0.5));

	vec2 image_size = 2.0 * canvas_box.zw;

	// compute the diagonale of the rectangle and the angle beta to the X Axis
	vec2 diag = image_size * 0.5;
	float     diag_length = length(diag);

	float beta = atan(diag.y, diag.x);

	// considering the ellipse contained by the rectangle
	// it is described by 
	//
	// | cos(t). W/2
	// | 
	// | sin(t). H/2
	// 
	// for t = beta, its length is				
	vec2 d = diag * vec2(cos(beta), sin(beta));

	float d_length = length(d);

	// the factor to apply to the ellipse so it fully contains the rectangle is
	float scale_factor = diag_length / d_length;

	// the radius = (Rx, Ry) is (start of the blend-out zone)
	vec2 min_radius = scale_factor * diag;

	// the radius for the end of the blend-out zone is (+10% radius)
	vec2 max_radius = scale_factor * (diag * blendzone_ratio);

	// compute for the considered point the angle with X axis
	vec2 center = image_size * 0.5;

	vec2 dp = pos - center;

	float alpha = (dp == vec2(0.0, 0.0)) ? 0.0 : atan(dp.y, dp.x);

	// compute the distant point on the ellipse
	vec2 a = vec2(cos(alpha), sin(alpha));

	vec2 min_limit = min_radius * a;
	vec2 max_limit = max_radius * a;

	// compute the distance from center to the 2 limits
	float min_distance = length(min_limit);
	float max_distance = length(max_limit);
	float p_distance = length(dp);

    // compute the decal 
    vec2 tex1 = 2.0 * vs_texcoord;
    vec2 tex2 = 2.0 * vs_texcoord;
    tex1.y += game_time * decal_speed;
    tex2.x += game_time * decal_speed;

    vec4 decal1 = texture(noise, tex1);
    vec4 decal2 = texture(noise, tex2);

    vec2 decal = -vec2(1.0, 1.0) + (decal1.xy + decal2.xy);

    // compute the blend value
	float ratio = 1.0 - fill_ratio;
	p_distance += ratio * max_distance;

	float value = 0.0;

	if (p_distance <= min_distance)
		value = 1.0;
	else if (p_distance >= max_distance)
		value = 0.0;
	else
		value = 1.0 - (p_distance - min_distance) / (max_distance - min_distance);

    // apply texture1 + texture2 + blend + noise
	vec2 texcoord = (vs_position.xy + vec2(1.0, 1.0)) * 0.5;

    float v1 = (1.0 - value);
    float v2 = value;

	vec4 clear_color  = texture(background_clear, texcoord + decal * v1 * v1);
	vec4 hidden_color = texture(background_hidden, texcoord + decal * v2 * v2);

	// mix both image textures
	vec4 mix_image = clear_color* v2 + hidden_color * v1;

	output_color = mix_image;
}