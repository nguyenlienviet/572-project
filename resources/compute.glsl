#version 440
#extension GL_ARB_shader_storage_buffer_object : require

#define CLOTHRESXY 200
#define NOMINALLENGTH 1.0

#define DAMP 0.001
#define SPRING 0.01

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

	float mypos = pos[u].y;
	float myvel = pos[u + CLOTHRESXY].y;
	//float myacc = pos[u + 2*CLOTHRESXY].y;
	float myacc = -1 * mypos * SPRING;

	for (int i = -1; i <= 1; i++)
	{
		if (i == 0 || u + i < 0 || u + i >= CLOTHRESXY) continue;

		float neighbor_pos = pos[u + i].y;
		float d = abs(neighbor_pos - mypos);
		float neighbor_acc = pos[(u + i) + (2 * CLOTHRESXY)].y;

		if (d != 0)
		{
			myacc -= neighbor_acc / d * 0.0001;
		}
	}

	//float myvel = -1 * (pos[(u - 1) + CLOTHRESXY].y + pos[(u + 1) + CLOTHRESXY].y) / 2;
	myvel += myacc * DAMP;

	pos[u].y += myvel;
	pos[u + CLOTHRESXY].y = myvel;
	pos[u + 2*CLOTHRESXY].y = myacc;


	//if(pos[u + v * CLOTHRESXY].y < -7) //is -7y is the ground
	//{
	//	pos[u + v * CLOTHRESXY].y = -7.0;
	//	pos[u + v * CLOTHRESXY + CLOTHRESXY*CLOTHRESXY].xyz = vec3(0,0,0);
	//}	
}
