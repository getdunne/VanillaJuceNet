# VanillaJuceNet

**VanillaJuceNet** illustrates how to wrap a juce::Synthesiser-derived synth into a simple JUCE-based DSP service, as described in [my ADC 2018 talk](https://juce.com/adc/programme/talks/networking).

This works together with my [VanillaJuce](https://github.com/getdunne/VanillaJuce) synth sample code. **Important:** when checking out the VanillaJuce project:
1. Make sure to check out the "network" branch, not the "master" branch.
2. Ensure that the *VanillaJuce* directory is a sibling of the *VanillaJuceNet* directory.

Refer to http://getdunne.net/wiki/doku.php?id=vanillajucenet for more details.

## Code licensing terms
This code is licensed under the terms of the MIT License (see the file *LICENSE* in this repo). To compile it, you will need a copy of the [JUCE framework](https://juce.com), and the resulting *combined work* will be subject to JUCE's own licensing terms, and under certain circumstances may become subject to the [GNU General Public License, version 3 (GPL3)](https://www.gnu.org/licenses/gpl-3.0.en.html).

I am grateful to Julian Storer of Roli, Inc. for clarifying, via the [JUCE Forum](https://forum.juce.com/t/open-source-without-gpl/29721), that this code will continue to be freely usable under the terms of the MIT license, because
1. The MIT license is "GPL3 compatible" according to the Free Software Foundation.
2. Clause 5(c) of the GPL3 specifically states that it does not invalidate less restrictive usage permissions explicitly granted by a copyright holder.
3. The JUCE license does not affect copyright status of MIT-licensed code in combined works.

In light of these considerations, and for maximum clarity, I have added my copyright notice and the full text of the MIT license to every one of the source files in this repo.

Shane Dunne, October 2018
