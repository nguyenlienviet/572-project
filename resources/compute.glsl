#version 440
#extension GL_ARB_shader_storage_buffer_object : require

#define CLOTHRESXY 20
#define NOMINALLENGTH 1.0

#define DAMP 0.0001
#define SPRING 0.0001

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

	vec3 mypos = pos[u].xyz;
	float myvel = pos[u + CLOTHRESXY].y;
	float myacc = pos[u + 2 * CLOTHRESXY].y;

	for (int i = -1; i <= 1; i++)
	{
		if (i == 0 || u + i < 0 || u + i >= CLOTHRESXY) continue;

		vec3 neighbor_pos = pos[u + i].xyz;
		float d = abs(neighbor_pos.y - mypos.y);
		float neighbor_acc = pos[(u + i) + (2 * CLOTHRESXY)].y;

		float corr = neighbor_acc / d * .00001;
		myacc += corr;
	}

	myacc += -1 * mypos.y * SPRING;
	myvel += myacc * DAMP;

	pos[u + CLOTHRESXY].y = myvel;
	pos[u + 2*CLOTHRESXY].y = myacc;
	pos[u].y += myvel;

	//if(pos[u + v * CLOTHRESXY].y < -7) //is -7y is the ground
	//{
	//	pos[u + v * CLOTHRESXY].y = -7.0;
	//	pos[u + v * CLOTHRESXY + CLOTHRESXY*CLOTHRESXY].xyz = vec3(0,0,0);
	//}	
}
