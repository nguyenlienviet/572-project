#version 440
#extension GL_ARB_shader_storage_buffer_object : require

#define CLOTHRESXY 200
#define NOMINALLENGTH 1.0

#define DAMP 1e-2
#define SPRING .02

layout(local_size_x = 1, local_size_y = 1) in;	 //sizex is number of cores

layout (std430, binding=0) restrict buffer VerticesVelocities
{ 
  vec4 pos[];
};

uniform float dx;
uniform float dy;


void main() 
{

	int u = int(gl_GlobalInvocationID.x);

	vec2 mypos = pos[u].xy;
	float myvel = pos[u + CLOTHRESXY].y;
	//float myacc = pos[u + 2*CLOTHRESXY].y;
	float myacc = -1 * mypos.y * SPRING;

	if (u == CLOTHRESXY/2) pos[u].y += dy;

	for (int i = -1; i <= 1; i++)
	{
		if (i == 0 || u + i < 0 || u + i >= CLOTHRESXY) continue;

		vec2 neighbor_pos = pos[u + i].xy;
		float y_dist = abs(neighbor_pos.y - mypos.y);
		//float dist = distance(neighbor_pos, mypos);

		myacc += DAMP * y_dist;
	}

	//float myvel = -1 * (pos[(u - 1) + CLOTHRESXY].y + pos[(u + 1) + CLOTHRESXY].y) / 2;
	myvel += myacc;

	pos[u].y += myvel;
	myvel *= .8;
	pos[u + CLOTHRESXY].y = myvel;
	//pos[u + 2*CLOTHRESXY].y = myacc;


	//if(pos[u + v * CLOTHRESXY].y < -7) //is -7y is the ground
	//{
	//	pos[u + v * CLOTHRESXY].y = -7.0;
	//	pos[u + v * CLOTHRESXY + CLOTHRESXY*CLOTHRESXY].xyz = vec3(0,0,0);
	//}	
}
