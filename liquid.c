#include "emscripten.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "include/liquid.h"

EMSCRIPTEN_KEEPALIVE
int version()
{
    return liquid_libversion_number();
}
EMSCRIPTEN_KEEPALIVE
int sgram()
{
    // options
    unsigned int nfft = 64;        // transform size
    unsigned int num_frames = 200; // total number of frames
    unsigned int msdelay = 50;     // delay between transforms [ms]
    float noise_floor = -40.0f;    // noise floor

    // initialize objects
    asgramcf q = asgramcf_create(nfft);
    asgramcf_set_scale(q, noise_floor + 15.0f, 5.0f);

    unsigned int i;
    unsigned int n;
    float theta = 0.0f;  // current instantaneous phase
    float dtheta = 0.0f; // current instantaneous frequency
    float phi = 0.0f;    // phase of sinusoidal frequency drift
    float dphi = 0.003f; // frequency of sinusoidal frequency drift

    float complex x[nfft];
    char ascii[nfft + 1];
    ascii[nfft] = '\0';                            // append null character to end of string
    float nstd = powf(10.0f, noise_floor / 20.0f); // noise standard deviation
    for (n = 0; n < num_frames; n++)
    {
        // generate a frame of data samples
        for (i = 0; i < nfft; i++)
        {
            // complex exponential
            x[i] = cexpf(_Complex_I * theta);

            // add noise to signal
            x[i] += nstd * (randnf() + _Complex_I * randnf()) * M_SQRT1_2;

            // adjust frequency and phase
            theta += dtheta;
            dtheta = 0.9f * M_PI * sinf(phi) * hamming(n, num_frames);
            phi += dphi;
        }

        // write block of samples to the spectrogram object
        asgramcf_write(q, x, nfft);

        // print result to screen
        asgramcf_print(q);

        // sleep for some time before generating the next frame
        usleep(msdelay * 1000);
    }

    asgramcf_destroy(q);
    printf("done.\n");
    return 0;
}
EMSCRIPTEN_KEEPALIVE
int ringBuffer() {
    float v[] = {1, 2, 3, 4, 5, 6, 7, 8};
    float *r; // reader
    unsigned int num_requested = 3;
    unsigned int num_read;

    cbufferf cb = cbufferf_create(10);

    cbufferf_write(cb, v, 4);
    cbufferf_read(cb, num_requested, &r, &num_read);
    printf("cbufferf: requested %u elements, read %u elements\n",
            num_requested, num_read);

    unsigned int i;
    for (i=0; i<num_read; i++)
        printf("  %u : %f\n", i, r[i]);

    // release 2 elements from the buffer
    cbufferf_release(cb, 2);

    // write 8 elements
    cbufferf_write(cb, v, 8);

    // print
    cbufferf_debug_print(cb);
    cbufferf_print(cb);

    // destroy object
    cbufferf_destroy(cb);

    printf("done.\n");
    return 0;
}
