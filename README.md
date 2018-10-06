# VanillaJuceNet

**VanillaJuceNet** illustrates how to wrap a juce::Synthesiser-derived synth into a simple JUCE-based DSP service, as described in [my ADC 2018 talk](https://juce.com/adc/programme/talks/networking).

This works together with my [VanillaJuce](https://github.com/getdunne/VanillaJuce) synth sample code. **Important:** when checking out the VanillaJuce project:
1. Make sure to check out the "network" branch, not the "master" branch.
2. Ensure that the *VanillaJuce* directory is a sibling of the *VanillaJuceNet* directory.

Refer to http://getdunne.net/wiki/doku.php?id=vanillajucenet for more details.

## Code licensing terms
This code is licensed under the terms of the MIT License (below, and also in the file *LICENSE* in this repo). To compile it, you will need a copy of the [JUCE framework](https://juce.com), and the resulting *combined work* will be subject to JUCE's own licensing terms.

It is my INTENT, as the author of this code, that everyone should have the right to use all or part of it in any JUCE-based program, and in the event that any such program should become subject to the GPL3 license (in accordance with the JUCE license terms), that this right should not be abridged. However, I doubt that this statement of intent would carry any weight under the law, so at this point I can only say: USE AT YOUR OWN RISK.

Shane Dunne, October 2018

> The MIT License (MIT)
> 
> Copyright (c) 2017-2018 Shane D. Dunne
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in
> all copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
> THE SOFTWARE.
