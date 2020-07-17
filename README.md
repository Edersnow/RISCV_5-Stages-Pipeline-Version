# RISC-V Simulator
An easy implementation of RISC-V simulator in C++.

It has passed the tests on [OJ](http://oj.oscardhc.com/).

**Concrete implementation**:
+ 5-stages pipeline
+ forwarding (deal with Data Hazard)
+ branch prediction (deal with Control Hazard)

In the prediction part, I use two kinds of dynamic predictors (local predictor and global predictor), and when the BHT hasn't got enough information, I use the BTFN (Back taken, forward not taken) strategy to predict. It's worth mentioning that the local predictor **(two-level adaptive predictor)** has the biggest impact on the accuracy.

The following table lists some of the prediction results (skip some tiny tests):

|Test|Correct/Total|Accuracy|
|:--:|:-----------:|-------:|
|basicopt1|154026/155139|99.28%|
|bulgarian|65074/71493|91.02%|
|hanoi|16747/17457|95.93%|
|magic|59272/67869|87.33%|
|pi|33734187/39956380|84.43%|
|qsort|195028/200045|97.49%|
|queens|62922/77116|81.59%|
|superloop|419424/435027|96.41%|
|tak|50231/60639|82.84%|

**PS**: In the prediction part, I found that the two-level adaptive predictor didn't perform well if it takes branches when the 2-bits predictor's value is 2 or 3, but when I happened to change the conditions into **"Only take branches when the value is 3"**, I found the awesome improvement in accurary (about 20% on average(?)). But I didn't found the same phenomenon in some other classmates (even if they use almost the same two-level adaptive predictor). I guess this may be attributed to the different structure of the simulator.

Reference & more information: [acm_wiki](https://acm.sjtu.edu.cn/wiki/PPCA_2020)