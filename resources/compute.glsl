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

layout (std430, binding=1) restrict buffer ObjectPos1
{ 
  vec4 objectPos1[];
};

layout (std430, binding=2) restrict buffer ObjectPos2
{ 
  vec4 objectPos2[];
};

layout (std430, binding=3) restrict buffer ObjectPos3
{ 
  vec4 objectPos3[];
};

layout (std430, binding=4) restrict buffer ObjectPos4
{ 
  vec4 objectPos4[];
};

uniform float dx;
uniform float dy;


void main() 
{
	int u = int(gl_GlobalInvocationID.x);

	if (u == 0)
	{
		objectPos1[0].y -= .02;
		objectPos1[1].y -= .02;
		objectPos1[2].y -= .02;
		objectPos1[3].y -= .02;

		objectPos2[0].y -= .02;
		objectPos2[1].y -= .02;
		objectPos2[2].y -= .02;
		objectPos2[3].y -= .02;

		objectPos3[0].y -= .02;
		objectPos3[1].y -= .02;
		objectPos3[2].y -= .02;
		objectPos3[3].y -= .02;

		objectPos4[0].y -= .02;
		objectPos4[1].y -= .02;
		objectPos4[2].y -= .02;
		objectPos4[3].y -= .02;
	}
	else
	{
		for (int i=0; i < 1000; i++)
		{
			float x = sqrt(1000);
		}
	}

	vec2 mypos = pos[u].xy;
	float myvel = pos[u + CLOTHRESXY].y;
	float myacc = -1 * mypos.y * SPRING;

	if (u == CLOTHRESXY/2) pos[u].y += dy;

	float leftBound1 = min(objectPos1[0].x, min(objectPos1[1].x, min(objectPos1[2].x, objectPos1[3].x)));
	float rightBound1 = max(objectPos1[0].x, max(objectPos1[1].x, max(objectPos1[2].x, objectPos1[3].x)));
	if (mypos.x >= leftBound1  && mypos.x <= rightBound1 && objectPos1[0].y >= -1 && objectPos1[0].y < mypos.y)
	{
		pos[u].y = objectPos1[0].y;
		return;
	}

	float leftBound2 = min(objectPos2[0].x, min(objectPos2[1].x, min(objectPos2[2].x, objectPos2[3].x)));
	float rightBound2 = max(objectPos2[0].x, max(objectPos2[1].x, max(objectPos2[2].x, objectPos2[3].x)));
	if (mypos.x >= leftBound2  && mypos.x <= rightBound2 && objectPos2[0].y >= -1 && objectPos2[0].y < mypos.y)
	{
		pos[u].y = objectPos2[0].y;
		return;
	}

	float leftBound3 = min(objectPos3[0].x, min(objectPos3[1].x, min(objectPos3[2].x, objectPos3[3].x)));
	float rightBound3 = max(objectPos3[0].x, max(objectPos3[1].x, max(objectPos3[2].x, objectPos3[3].x)));
	if (mypos.x >= leftBound3  && mypos.x <= rightBound3 && objectPos3[0].y >= -1 && objectPos3[0].y < mypos.y)
	{
		pos[u].y = objectPos3[0].y;
		return;
	}

	float leftBound4 = min(objectPos4[0].x, min(objectPos4[1].x, min(objectPos4[2].x, objectPos4[3].x)));
	float rightBound4 = max(objectPos4[0].x, max(objectPos4[1].x, max(objectPos4[2].x, objectPos4[3].x)));
	if (mypos.x >= leftBound4  && mypos.x <= rightBound4 && objectPos4[0].y >= -1 && objectPos4[0].y < mypos.y)
	{
		pos[u].y = objectPos4[0].y;
		return;
	}

	for (int i = -1; i <= 1; i++)
	{
		if (i == 0 || u + i < 0 || u + i >= CLOTHRESXY) continue;

		vec2 neighbor_pos = pos[u + i].xy;
		float y_dist = (neighbor_pos.y - mypos.y);
		//float dist = distance(neighbor_pos, mypos);

		myacc += DAMP * y_dist;
	}

	myvel += myacc;

	pos[u].y += myvel;
	myvel *= .95;
	pos[u + CLOTHRESXY].y = myvel;

	//if(pos[u + v * CLOTHRESXY].y < -7) //is -7y is the ground
	//{
	//	pos[u + v * CLOTHRESXY].y = -7.0;
	//	pos[u + v * CLOTHRESXY + CLOTHRESXY*CLOTHRESXY].xyz = vec3(0,0,0);
	//}	
}
