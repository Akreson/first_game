internal void
CompileBitmapProgram(bitmap_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec3 Vertex;
	layout (location = 1) in vec3 Color;
	layout (location = 2) in vec2 TextCoord;

	out vec2 TexCoords;
	
	uniform mat4 Proj;

	void main()
	{
		TexCoords = TextCoord;
		gl_Position = Proj * vec4(Vertex, 1.0f);
	}

	)FOO";
	const char *FragmentCode = R"FOO(
	in vec2 TexCoords;	
	out vec4 FragColor;

	uniform sampler2D BitmapTexture;
	uniform vec3 Color;
	
	void main()
	{
		vec4 Texel = texture(BitmapTexture, TexCoords);
		Texel.rgb *= Color;
		FragColor = Texel;
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);

	Prog->ColorID = glGetUniformLocation(ProgID, "Color");
	Prog->ProjID = glGetUniformLocation(ProgID, "Proj");
	glUseProgram(0);
}

internal void
UseProgramBegin(bitmap_program *Prog, v3 Color, m4x4 *ProgMat)
{
	glUseProgram(Prog->ID);

	glUniform3f(Prog->ColorID, Color.r, Color.g, Color.b);
	glUniformMatrix4fv(Prog->ProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
}

internal void
UseProgramEnd(bitmap_program *Prog)
{
	glUseProgram(0);
}

// TODO: Fix bug with colored non selected edge for
// small view degree on non slected face
internal void
CompileModelProgram(model_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec3 aBarCoord;
	layout (location = 2) in vec3 aActiveMask;
	layout (location = 3) in vec3 aHotMask;
	layout (location = 4) in vec2 aFaceSelectionParam;

	uniform mat4 Proj;
	uniform mat4 ModelTransform;

	out vec3 BarCoord;
	flat out vec3 HotMask;
	flat out vec3 ActiveMask;
	flat out vec2 FaceSelectionParam;	

	void main()
	{
		BarCoord = aBarCoord;
		HotMask = aHotMask;
		ActiveMask = aActiveMask;
		FaceSelectionParam = aFaceSelectionParam;

		gl_Position = Proj * ModelTransform * vec4(aPos.xyz, 1.0f);
	}

	)FOO";

	// TODO: Set model color?
	// TODO: Compute color for selected edge in right way

	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	uniform vec4 Color;
	uniform vec3 EdgeColor;

	in vec3 BarCoord;
	flat in vec3 HotMask;
	flat in vec3 ActiveMask;
	flat in vec2 FaceSelectionParam;
	
	struct EdgeParams
	{
		float Factor;
		uint MinIndex;
	};

	float WhenEq(float x, float y) {
		return 1.0 - abs(sign(x - y));
	}
	
	float  WhenNeq(float x, float y) {
		return abs(sign(x - y));
	}

#if 1
	EdgeParams GetEdgeParams(vec3 A, float Thickness)
	{
		EdgeParams Result;
		
		float MinD = min(min(A.x, A.y), A.z);

		float dMinD = fwidth(MinD);
		Result.Factor = smoothstep(0, Thickness*dMinD, MinD);
		
		float Index = 0;
		Index += 1.0 * WhenEq(MinD, A.y);
		Index += 2.0 * WhenEq(MinD, A.z);
		Result.MinIndex = int(Index);

		return Result;
	}
#else
	// TODO: Complete
	EdgeParams GetEdgeParams(vec3 A, float Thickness)
	{
		EdgeParams Result;
		vec3 StoredA = A;

		float MinD0 = min(min(StoredA.x, StoredA.y), StoredA.z);
		float dMinD0 = fwidth(MinD0);
		float Factor0 = smoothstep(0, Thickness*dMinD0, MinD0);

		float Index0 = 0;
		Index0 += 1.0 * WhenEq(MinD0, StoredA.y);
		Index0 += 2.0 * WhenEq(MinD0, StoredA.z);
		uint IntMinIndex0 = int(Index0);

		StoredA[Result.MinIndex] = 1.0f;

		float MinD1 = min(min(StoredA.x, StoredA.y), StoredA.z);
		float dMinD1 = fwidth(MinD1);
		float Factor1 = smoothstep(0, Thickness*dMinD1, MinD1);

		float Index1 = 0;
		Index1 += 1.0 * WhenEq(MinD1, A.y);
		Index1 += 2.0 * WhenEq(MinD1, A.z);
		uint IntMinIndex1 = int(Index1);

		float MaxFactor = max(Factor0, Factor1);
		float FinalFactor = mix(min(Factor0, Factor1), MaxFactor, WhenNeq(MaxFactor, 1.0f));
		
		float FinalIndex = Index0 * WhenEq(Factor0, FinalFactor);
		FinalIndex = Index1 * WhenEq(Factor1, FinalFactor);
		uint IntFinalIndex = int(FinalIndex);
		
		Result.MinIndex = IntFinalIndex;
		Result.Factor = FinalFactor;

		return Result;
	}
#endif

	void main()
	{
		//vec3 _EdgeColor = vec3(0.17f, 0.5f, 0.8f); // NOTE: For Debug

		vec3 ActiveColor = vec3(0.86f, 0.65f, 0.2f);
		vec3 HotFaceColor = vec3(1.6f);
		float Thickness = 1.5f;
	
		// NOTE: Edge color calc.
		EdgeParams Edge = GetEdgeParams(BarCoord, Thickness);
		float InvEdgeFactor = 1.0f - Edge.Factor;
		
		float ActiveEdgeFactor = ActiveMask[Edge.MinIndex];
		float HotEdgeFactor = HotMask[Edge.MinIndex];

		vec3 FinalEdgeColor = mix(EdgeColor, ActiveColor, ActiveEdgeFactor);
		FinalEdgeColor = mix(FinalEdgeColor, FinalEdgeColor*HotFaceColor, HotEdgeFactor);
		
		// NOTE: Face color calc
		vec3 FinalFaceColor = mix(Color.rgb, (ActiveColor*Color.rgb), FaceSelectionParam.x);
		FinalFaceColor = mix(FinalFaceColor, (FinalFaceColor*HotFaceColor), FaceSelectionParam.y);

		FragColor = vec4(mix(FinalFaceColor, FinalEdgeColor, InvEdgeFactor), Color.a);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	Prog->ModelColorID = glGetUniformLocation(ProgID, "Color");
	Prog->ModelProjID = glGetUniformLocation(ProgID, "Proj");
	Prog->ModelTransformID = glGetUniformLocation(ProgID, "ModelTransform");

	// TODO: Use for debuging, delete later
	Prog->ModelEdgeColor = glGetUniformLocation(ProgID, "EdgeColor");

	glGenVertexArrays(1, &Prog->ModelVAO);
	glGenBuffers(1, &Prog->ModelVBO);

	glUseProgram(0);
}

internal void
UseProgramBegin(model_program *Prog, v4 Color, v3 EdgeColor, m4x4 *ProgMat, m4x4 *ModelMat)
{
	glUseProgram(Prog->ID);

	glUniform4f(Prog->ModelColorID, Color.r, Color.g, Color.b, Color.a);
	glUniformMatrix4fv(Prog->ModelProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
	glUniformMatrix4fv(Prog->ModelTransformID, 1, GL_FALSE, &ModelMat->E[0][0]);

	// TODO: Delete later
	glUniform3f(Prog->ModelEdgeColor, EdgeColor.r, EdgeColor.g, EdgeColor.b);

}

internal void
UseProgramEnd(model_program *Prog)
{
	glUseProgram(0);
}

// NOTE: It's not exactly right set outline color here, look just ok.
// TODO: Find to set in proper way outline color in outline_program?
void
CompileModelColorPassProgram(model_color_pass_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec4 aPos;
	layout (location = 1) in vec4 aBarCoord;

	uniform mat4 Proj;
	uniform mat4 ModelTransform;

	void main()
	{
		gl_Position = Proj * ModelTransform * vec4(aPos.xyz, 1.0f);
	}
	)FOO";

	const char *FragmentCode = R"FOO(
	out vec4 FragColor;
	
	void main()
	{
		FragColor = vec4(1.0f);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	Prog->ModelProjID = glGetUniformLocation(ProgID, "Proj");
	Prog->ModelTransformID = glGetUniformLocation(ProgID, "ModelTransform");
	glUseProgram(0);
}

internal void
UseProgramBegin(model_color_pass_program *Prog, m4x4 *ProgMat, m4x4 *ModelMat)
{
	glUseProgram(Prog->ID);

	glUniformMatrix4fv(Prog->ModelProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
	glUniformMatrix4fv(Prog->ModelTransformID, 1, GL_FALSE, &ModelMat->E[0][0]);
}

internal void
UseProgramEnd(model_color_pass_program *Prog)
{
	glUseProgram(0);
}

internal void
CompileBlurProgram(blur_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec2 Vertex;
	layout (location = 1) in vec2 TextCoord;

	out vec2 TexCoords;

	void main()
	{
		TexCoords = TextCoord;
		gl_Position = vec4(Vertex, 0, 1.0);
	}

	)FOO";
	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	in vec2 TexCoords;	

	uniform sampler2D Texture;
	
	void main()
	{
		vec2 TexelSize = 1.0f / textureSize(Texture, 0);
		
		float Color = texture(Texture, TexCoords).r;
		Color += texture(Texture, TexCoords + (vec2(-1.0f, 1.0f) * TexelSize)).r;
		Color += texture(Texture, TexCoords + (vec2(1.0f, 1.0f) * TexelSize)).r;
		Color += texture(Texture, TexCoords + (vec2(1.0f, -1.0f) * TexelSize)).r;
		Color += texture(Texture, TexCoords + (vec2(-1.0f, -1.0f) * TexelSize)).r;

		FragColor = vec4(Color / 5.0f, 0, 0, 1.0f);
	}
	)FOO";

	Prog->ID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
}

internal void
UseProgramBegin(blur_program *Prog)
{
	glUseProgram(Prog->ID);
}

internal void
UseProgramEnd(blur_program *Prog)
{
	glUseProgram(0);
}

void
CompileOutlinePassProgram(outline_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec2 Vertex;
	layout (location = 1) in vec2 TextCoord;

	out vec2 TexCoords;

	void main()
	{
		TexCoords = TextCoord;
		gl_Position = vec4(Vertex, 0, 1.0f);
	}

	)FOO";
	// TODO: Set default framebuffer for
	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	in vec2 TexCoords;	

	uniform sampler2D PrepassTex;
	uniform sampler2D BlurTex;
	uniform sampler2D MainTex;
	uniform vec3 OutlineColor;
	
	void main()
	{
		vec3 MainColor = texture(MainTex, TexCoords).rgb;
		float PrepassColor = texture(PrepassTex, TexCoords).r;
		float BlurColor = texture(BlurTex, TexCoords).r;
		
		float DiffColor = max(0, BlurColor - PrepassColor);
		
		FragColor = vec4(MainColor + (vec3(DiffColor * 1.2) * OutlineColor), 1.0f);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	glUniform1i(glGetUniformLocation(ProgID, "PrepassTex"), 0);
	glUniform1i(glGetUniformLocation(ProgID, "BlurTex"), 1);
	glUniform1i(glGetUniformLocation(ProgID, "MainTex"), 2);
	Prog->OutlineColor = glGetUniformLocation(ProgID, "OutlineColor");
	glUseProgram(0);
}

internal void
UseProgramBegin(outline_program *Prog, v3 Color)
{
	glUseProgram(Prog->ID);
	glUniform3f(Prog->OutlineColor, Color.r, Color.g, Color.b);
}

internal void
UseProgramEnd(outline_program *Prog)
{
	glUseProgram(0);
}

internal void
CompileStaticMeshProgram(static_mesh_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec3 aPos;

	uniform mat4 Proj;
	uniform mat4 ModelTransform;

	void main()
	{
		gl_Position = Proj * ModelTransform * vec4(aPos, 1.0f);
	}

	)FOO";

	// TODO: Set model color?
	// TODO: Compute color for selected edge in right way

	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	uniform vec3 MeshColor;
	
	void main()
	{
		FragColor = vec4(MeshColor, 1.0);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	Prog->ProjID = glGetUniformLocation(ProgID, "Proj");
	Prog->TransformID = glGetUniformLocation(ProgID, "ModelTransform");
	Prog->ColorID = glGetUniformLocation(ProgID, "MeshColor");
	glUseProgram(0);
}

internal void
UseProgramBegin(static_mesh_program *Prog, m4x4 *ProgMat, m4x4 *ModelMat, v3 Color)
{
	glUseProgram(Prog->ID);

	glUniformMatrix4fv(Prog->ProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
	glUniformMatrix4fv(Prog->TransformID, 1, GL_FALSE, &ModelMat->E[0][0]);
	glUniform3f(Prog->ColorID, Color.r, Color.g, Color.b);
}

internal void
UseProgramEnd(static_mesh_program *Prog)
{
	glUseProgram(0);
}

// TODO: Make alpha work in proper way?
internal void
CompileRotateToolProgram(rotate_tool_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec3 aPos;

	uniform mat4 PersProj;
	uniform mat4 CameraTransform;
	uniform mat4 ModelTransform;

	out vec3 PointOnSphere;
	out vec3 WorldPointOnSphere;

	void main()
	{
		PointOnSphere = aPos;
		vec4 WordPos = ModelTransform * vec4(aPos, 1.0f);
		WorldPointOnSphere = WordPos.xyz;
		gl_Position = PersProj * CameraTransform * WordPos;
	}

	)FOO";
	
	const char *FragmentCode = R"FOO(
	#define RED 0 
	#define GREEN 1
	#define BLUE 2
	#define MAX_FILL_THRESHOLD 0.2f

	out vec4 FragColor;

	uniform vec3 XAxis;
	uniform vec3 YAxis;
	uniform vec3 ZAxis;
	uniform vec3 CenterPos;
	uniform vec3 ViewDir;
	uniform vec4 AxisState; // NOTE: _w_ - active or not
	uniform ivec2 PerpInfo; // NOTE: x - index, y - is perp axis set
	
	
	in vec3 PointOnSphere;
	in vec3 WorldPointOnSphere;

	float WhenEq(float x, float y) {
		return 1.0f - abs(sign(x - y));
	}
	
	float WhenNeq(float x, float y) {
		return abs(sign(x - y));
	}

	float WhenLt(float x, float y) {
		return max(sign(y - x), 0.0f);
	}

	float WhenGt(float x, float y) {
		return max(sign(x - y), 0.0f);
	}

	float IsAxisPerp(float Index, float PerpAxisIndex, float IsPerpAxisSet)
	{
		float SetMask = WhenEq(IsPerpAxisSet, 0.0f);
		float InvSetMask = 1.0f - SetMask;
		float Result = SetMask + (InvSetMask * (1.0f - WhenEq(PerpAxisIndex, Index)));
		return Result;
	}

	void main()
	{
		float Thickness = 0.02f;
		vec3 AxisColor[3];
		float PerpAxisIndex = float(PerpInfo.x);
		float IsPerpAxisSet = float(PerpInfo.y);

		AxisColor[RED] = mix(vec3(0.6f, 0, 0), vec3(1.0f, 0, 0), AxisState.x);
		AxisColor[GREEN] = mix(vec3(0, 0.6f, 0), vec3(0, 1.0f, 0), AxisState.y);
		AxisColor[BLUE] = mix(vec3(0, 0, 0.6f), vec3(0, 0, 1.0f), AxisState.z);
		
		AxisColor[RED] = mix(AxisColor[RED], vec3(0.86f, 0.65f, 0.2f), AxisState.x*AxisState.w);
		AxisColor[GREEN] = mix(AxisColor[GREEN], vec3(0.86f, 0.65f, 0.2f), AxisState.y*AxisState.w);
		AxisColor[BLUE] = mix(AxisColor[BLUE], vec3(0.86f, 0.65f, 0.2f), AxisState.z*AxisState.w);
		
		vec3 DirFromCenter = normalize(PointOnSphere);
		float XDotP = abs(dot(XAxis.xyz, DirFromCenter));
		float YDotP = abs(dot(YAxis.xyz, DirFromCenter));
		float ZDotP = abs(dot(ZAxis.xyz, DirFromCenter));

		float XModifier = IsAxisPerp(0, PerpAxisIndex, IsPerpAxisSet);
		float YModifier = IsAxisPerp(1, PerpAxisIndex, IsPerpAxisSet);
		float ZModifier = IsAxisPerp(2, PerpAxisIndex, IsPerpAxisSet);

		float XAlpha = WhenLt(XDotP, Thickness) * XModifier;
		float YAlpha = WhenLt(YDotP, Thickness) * YModifier;
		float ZAlpha = WhenLt(ZDotP, Thickness) * ZModifier;
		float FinalAlpha = max(XAlpha, max(YAlpha, ZAlpha));		

		vec3 FinalColor = ((AxisColor[RED] * XAlpha) * (1.0f - YAlpha) * (1.0f - ZAlpha));
		FinalColor += ((AxisColor[GREEN] * YAlpha) * (1.0f - ZAlpha));
		FinalColor += (AxisColor[BLUE] * ZAlpha);
		
		// TODO: Check for non zero origin
		// TODO: Get rid of branch

		float DotResult = dot(ViewDir, DirFromCenter);
		if (DotResult <= MAX_FILL_THRESHOLD)
		{
			FinalColor = vec3(0, 0, 0);
			FinalAlpha = 1.0f;
			if (PerpInfo.y == 1)
			{
				FinalColor = AxisColor[PerpInfo.x];
			}
		}

		FragColor = vec4(FinalColor, FinalAlpha);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	Prog->PersProj = glGetUniformLocation(ProgID, "PersProj");
	Prog->ModelTransform = glGetUniformLocation(ProgID, "ModelTransform");
	Prog->CameraTransform = glGetUniformLocation(ProgID, "CameraTransform");
	Prog->XAxis = glGetUniformLocation(ProgID, "XAxis");
	Prog->YAxis = glGetUniformLocation(ProgID, "YAxis");
	Prog->ZAxis = glGetUniformLocation(ProgID, "ZAxis");
	Prog->AxisActivityState = glGetUniformLocation(ProgID, "AxisState");
	Prog->CenterPos = glGetUniformLocation(ProgID, "CenterPos");
	Prog->ViewDir = glGetUniformLocation(ProgID, "ViewDir");
	Prog->PerpInfo = glGetUniformLocation(ProgID, "PerpInfo");
	glUseProgram(0);
}

internal void
UseProgramBegin(rotate_tool_program *Prog, m4x4 *ProgMat, m4x4 *CameraMat,
	m4x4 *ModelMat, render_entry_tool_rotate *Tools)
{
	glUseProgram(Prog->ID);

	glUniformMatrix4fv(Prog->PersProj, 1, GL_FALSE, &ProgMat->E[0][0]);
	glUniformMatrix4fv(Prog->CameraTransform, 1, GL_FALSE, &CameraMat->E[0][0]);
	glUniformMatrix4fv(Prog->ModelTransform, 1, GL_FALSE, &ModelMat->E[0][0]);

	glUniform3fv(Prog->XAxis, 1, (const GLfloat *)&Tools->XAxis);
	glUniform3fv(Prog->YAxis, 1, (const GLfloat *)&Tools->YAxis);
	glUniform3fv(Prog->ZAxis, 1, (const GLfloat *)&Tools->ZAxis);
	glUniform4fv(Prog->AxisActivityState, 1, (const GLfloat *)&Tools->AxisActivityState);
	glUniform3fv(Prog->CenterPos, 1, (const GLfloat *)&Tools->Pos);
	glUniform3fv(Prog->ViewDir, 1, (const GLfloat *)&Tools->ViewDir);
	glUniform2i(Prog->PerpInfo, Tools->PerpInfo.x, Tools->PerpInfo.y);
}

internal void
UseProgramEnd(rotate_tool_program *Prog)
{
	glUseProgram(0);
}