#version 150
in vec4 fColor;
in vec2 fUV1;
in vec2 fUV2;

out vec4 oColor;

// Built-in uniforms
uniform float gAlphaTest;

// Uniforms
uniform sampler2D uDiffuse;
uniform float uDiffuseVisible = 1.0f; // 1 = diffuse visible, 0 = only lightmap visible

uniform sampler2D uLightmap;
uniform float uLightmapMultiplier = 2.0f;

void main()
{
    // Grab color texel. Discard if below alpha test value.
    vec4 texel = texture(uDiffuse, fUV1);
	if(texel.a < gAlphaTest) { discard; }

    // Show/hide diffuse texture. May want to show just lightmap texture for debugging.
    texel = (uDiffuseVisible * texel) + ((1.0f - uDiffuseVisible) * vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Grab lightmap texel; apply multiplier.
    vec4 lightmapTexel = texture(uLightmap, fUV2);
    lightmapTexel.rgb *= uLightmapMultiplier;

    // Create final output color.
    oColor = texel * lightmapTexel;
}
