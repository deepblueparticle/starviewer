// per fer-ho amb les proporcions correctes:
// * fer el paral·lelepípede de la mida adequada -> canviar els vèrtexs
// * mantenir els colors com fins ara -> la interpolació ens donarà les coordenades de textura
// * start_coord serà el color d'aquesta segona passada
// * end_coord serà el color del backbuffer al pìxel actual
// * necessitem dimX, dimY, dimZ com a uniforms (vec3)
// * escalem start_coord i end_coord amb dims per saber els punts amb mides reals
// - calculem la direcció i el pas amb aquestes mides reals
// - escalem el pas entre 0 i 1 en cada dimensió
// - ...
// - PROFIT!!!


uniform sampler2D uFramebufferTexture;
uniform vec3 uDimensions;
uniform sampler3D uVolumeTexture;

// IN.TexCoord -> gl_TexCoord[0]
// IN.Color -> gl_TexCoord[1]
// IN.Pos -> gl_TexCoord[2]

void main()
{
    const float STEP_SIZE = 1.0;
    const float OPAQUE_ALPHA = 0.9;

    vec3 startCoord = gl_Color.rgb;
    vec2 framebufferCoord = ((gl_TexCoord[2].xy / gl_TexCoord[2].w) + 1.0) / 2.0;   // no sé ben bé d'on baixa això però funciona
    vec3 endCoord = texture2D(uFramebufferTexture, framebufferCoord).rgb;
    vec3 startPoint = startCoord * uDimensions;
    vec3 endPoint = endCoord * uDimensions;
    vec3 direction = normalize(endPoint - startPoint);
    vec3 pointStep = direction * STEP_SIZE;
    vec3 coordStep = pointStep / uDimensions;

    vec4 color = vec4(0.0);
    float remainingOpacity = 1.0;
    vec3 coord = startCoord;

    for (int i = 0; i < 512; i++)
    {
        vec4 colorSample = texture3D(uVolumeTexture, coord);
        color.rgb += colorSample.rgb * colorSample.a * remainingOpacity;
        remainingOpacity *= 1.0 - colorSample.a;
        color.a = 1.0 - remainingOpacity;

        if (color.a >= OPAQUE_ALPHA) break;

        coord += coordStep;

        if (any(lessThan(coord, vec3(0.0))) || any(greaterThan(coord, vec3(1.0)))) break;
    }

    gl_FragColor = color;
}
