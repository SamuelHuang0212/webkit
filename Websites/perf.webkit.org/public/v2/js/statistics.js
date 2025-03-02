var Statistics = new (function () {

    this.min = function (values) {
        return Math.min.apply(Math, values);
    }

    this.max = function (values) {
        return Math.max.apply(Math, values);
    }

    this.sum = function (values) {
        return values.length ? values.reduce(function (a, b) { return a + b; }) : 0;
    }

    this.squareSum = function (values) {
        return values.length ? values.reduce(function (sum, value) { return sum + value * value;}, 0) : 0;
    }

    // With sum and sum of squares, we can compute the sample standard deviation in O(1).
    // See https://rniwa.com/2012-11-10/sample-standard-deviation-in-terms-of-sum-and-square-sum-of-samples/
    this.sampleStandardDeviation = function (numberOfSamples, sum, squareSum) {
        if (numberOfSamples < 2)
            return 0;
        return Math.sqrt(squareSum / (numberOfSamples - 1) - sum * sum / (numberOfSamples - 1) / numberOfSamples);
    }

    this.supportedConfidenceIntervalProbabilities = function () {
        var supportedProbabilities = [];
        for (var quantile in tDistributionQuantiles)
            supportedProbabilities.push((1 - (1 - quantile) * 2).toFixed(2));
        return supportedProbabilities
    }

    // Computes the delta d s.t. (mean - d, mean + d) is the confidence interval with the specified probability in O(1).
    this.confidenceIntervalDelta = function (probability, numberOfSamples, sum, squareSum) {
        var quantile = (1 - (1 - probability) / 2);
        if (!(quantile in tDistributionQuantiles)) {
            throw 'We only support ' + this.supportedConfidenceIntervalProbabilities().map(function (probability)
            { return probability * 100 + '%'; } ).join(', ') + ' confidence intervals.';
        }
        if (numberOfSamples - 2 < 0)
            return NaN;
        var deltas = tDistributionQuantiles[quantile];
        var degreesOfFreedom = numberOfSamples - 1;
        if (degreesOfFreedom > deltas.length)
            throw 'We only support up to ' + deltas.length + ' degrees of freedom';

        // d = c * S/sqrt(numberOfSamples) where c ~ t-distribution(degreesOfFreedom) and S is the sample standard deviation.
        return deltas[degreesOfFreedom - 1] * this.sampleStandardDeviation(numberOfSamples, sum, squareSum) / Math.sqrt(numberOfSamples);
    }

    this.confidenceInterval = function (values, probability) {
        var sum = this.sum(values);
        var mean = sum / values.length;
        var delta = this.confidenceIntervalDelta(probability || 0.95, values.length, sum, this.squareSum(values));
        return [mean - delta, mean + delta];
    }

    var tDistributionQuantiles = {
        0.9: [
            3.077684, 1.885618, 1.637744, 1.533206, 1.475884, 1.439756, 1.414924, 1.396815, 1.383029, 1.372184,
            1.363430, 1.356217, 1.350171, 1.345030, 1.340606, 1.336757, 1.333379, 1.330391, 1.327728, 1.325341,
            1.323188, 1.321237, 1.319460, 1.317836, 1.316345, 1.314972, 1.313703, 1.312527, 1.311434, 1.310415,
            1.309464, 1.308573, 1.307737, 1.306952, 1.306212, 1.305514, 1.304854, 1.304230, 1.303639, 1.303077,
            1.302543, 1.302035, 1.301552, 1.301090, 1.300649, 1.300228, 1.299825, 1.299439, 1.299069, 1.298714,

            1.298373, 1.298045, 1.297730, 1.297426, 1.297134, 1.296853, 1.296581, 1.296319, 1.296066, 1.295821,
            1.295585, 1.295356, 1.295134, 1.294920, 1.294712, 1.294511, 1.294315, 1.294126, 1.293942, 1.293763,
            1.293589, 1.293421, 1.293256, 1.293097, 1.292941, 1.292790, 1.292643, 1.292500, 1.292360, 1.292224,
            1.292091, 1.291961, 1.291835, 1.291711, 1.291591, 1.291473, 1.291358, 1.291246, 1.291136, 1.291029,
            1.290924, 1.290821, 1.290721, 1.290623, 1.290527, 1.290432, 1.290340, 1.290250, 1.290161, 1.290075],
        0.95: [
            6.313752, 2.919986, 2.353363, 2.131847, 2.015048, 1.943180, 1.894579, 1.859548, 1.833113, 1.812461,
            1.795885, 1.782288, 1.770933, 1.761310, 1.753050, 1.745884, 1.739607, 1.734064, 1.729133, 1.724718,
            1.720743, 1.717144, 1.713872, 1.710882, 1.708141, 1.705618, 1.703288, 1.701131, 1.699127, 1.697261,
            1.695519, 1.693889, 1.692360, 1.690924, 1.689572, 1.688298, 1.687094, 1.685954, 1.684875, 1.683851,
            1.682878, 1.681952, 1.681071, 1.680230, 1.679427, 1.678660, 1.677927, 1.677224, 1.676551, 1.675905,

            1.675285, 1.674689, 1.674116, 1.673565, 1.673034, 1.672522, 1.672029, 1.671553, 1.671093, 1.670649,
            1.670219, 1.669804, 1.669402, 1.669013, 1.668636, 1.668271, 1.667916, 1.667572, 1.667239, 1.666914,
            1.666600, 1.666294, 1.665996, 1.665707, 1.665425, 1.665151, 1.664885, 1.664625, 1.664371, 1.664125,
            1.663884, 1.663649, 1.663420, 1.663197, 1.662978, 1.662765, 1.662557, 1.662354, 1.662155, 1.661961,
            1.661771, 1.661585, 1.661404, 1.661226, 1.661052, 1.660881, 1.660715, 1.660551, 1.660391, 1.660234],
        0.975: [
            12.706205, 4.302653, 3.182446, 2.776445, 2.570582, 2.446912, 2.364624, 2.306004, 2.262157, 2.228139,
            2.200985, 2.178813, 2.160369, 2.144787, 2.131450, 2.119905, 2.109816, 2.100922, 2.093024, 2.085963,
            2.079614, 2.073873, 2.068658, 2.063899, 2.059539, 2.055529, 2.051831, 2.048407, 2.045230, 2.042272,
            2.039513, 2.036933, 2.034515, 2.032245, 2.030108, 2.028094, 2.026192, 2.024394, 2.022691, 2.021075,
            2.019541, 2.018082, 2.016692, 2.015368, 2.014103, 2.012896, 2.011741, 2.010635, 2.009575, 2.008559,

            2.007584, 2.006647, 2.005746, 2.004879, 2.004045, 2.003241, 2.002465, 2.001717, 2.000995, 2.000298,
            1.999624, 1.998972, 1.998341, 1.997730, 1.997138, 1.996564, 1.996008, 1.995469, 1.994945, 1.994437,
            1.993943, 1.993464, 1.992997, 1.992543, 1.992102, 1.991673, 1.991254, 1.990847, 1.990450, 1.990063,
            1.989686, 1.989319, 1.988960, 1.988610, 1.988268, 1.987934, 1.987608, 1.987290, 1.986979, 1.986675,
            1.986377, 1.986086, 1.985802, 1.985523, 1.985251, 1.984984, 1.984723, 1.984467, 1.984217, 1.983972],
        0.99: [
            31.820516, 6.964557, 4.540703, 3.746947, 3.364930, 3.142668, 2.997952, 2.896459, 2.821438, 2.763769,
            2.718079, 2.680998, 2.650309, 2.624494, 2.602480, 2.583487, 2.566934, 2.552380, 2.539483, 2.527977,
            2.517648, 2.508325, 2.499867, 2.492159, 2.485107, 2.478630, 2.472660, 2.467140, 2.462021, 2.457262,
            2.452824, 2.448678, 2.444794, 2.441150, 2.437723, 2.434494, 2.431447, 2.428568, 2.425841, 2.423257,
            2.420803, 2.418470, 2.416250, 2.414134, 2.412116, 2.410188, 2.408345, 2.406581, 2.404892, 2.403272,

            2.401718, 2.400225, 2.398790, 2.397410, 2.396081, 2.394801, 2.393568, 2.392377, 2.391229, 2.390119,
            2.389047, 2.388011, 2.387008, 2.386037, 2.385097, 2.384186, 2.383302, 2.382446, 2.381615, 2.380807,
            2.380024, 2.379262, 2.378522, 2.377802, 2.377102, 2.376420, 2.375757, 2.375111, 2.374482, 2.373868,
            2.373270, 2.372687, 2.372119, 2.371564, 2.371022, 2.370493, 2.369977, 2.369472, 2.368979, 2.368497,
            2.368026, 2.367566, 2.367115, 2.366674, 2.366243, 2.365821, 2.365407, 2.365002, 2.364606, 2.364217]
    };

    this.MovingAverageStrategies = [
        {
            id: 1,
            label: 'Simple Moving Average',
            parameterList: [
                {label: "Backward window size", value: 8, min: 2, step: 1},
                {label: "Forward window size", value: 4, min: 0, step: 1}
            ],
            execute: function (backwardWindowSize, forwardWindowSize, values) {
                var averages = new Array(values.length);
                // We use naive O(n^2) algorithm for simplicy as well as to avoid accumulating round-off errors.
                for (var i = 0; i < values.length; i++) {
                    var sum = 0;
                    var count = 0;
                    for (var j = i - backwardWindowSize; j < i + backwardWindowSize; j++) {
                        if (j >= 0 && j < values.length) {
                            sum += values[j];
                            count++;
                        }
                    }
                    averages[i] = sum / count;
                }
                return averages;
            },

        },
        {
            id: 2,
            label: 'Cumulative Moving Average',
            execute: function (values) {
                var averages = new Array(values.length);
                var sum = 0;
                for (var i = 0; i < values.length; i++) {
                    sum += values[i];
                    averages[i] = sum / (i + 1);
                }
                return averages;
            }
        },
        {
            id: 3,
            label: 'Exponential Moving Average',
            parameterList: [{label: "Smoothing factor", value: 0.1, min: 0.001, max: 0.9}],
            execute: function (smoothingFactor, values) {
                if (!values.length || typeof(smoothingFactor) !== "number")
                    return null;

                var averages = new Array(values.length);
                var movingAverage = 0;
                averages[0] = values[0];
                for (var i = 1; i < values.length; i++)
                    averages[i] = smoothingFactor * values[i] + (1 - smoothingFactor) * averages[i - 1];
                return averages;
            }
        },
    ];

    this.EnvelopingStrategies = [
        {
            id: 100,
            label: 'Average Difference',
            description: 'The average difference between consecutive values.',
            execute: function (values, movingAverages) {
                if (values.length < 1)
                    return NaN;

                var diff = 0;
                for (var i = 1; i < values.length; i++)
                    diff += Math.abs(values[i] - values[i - 1]);

                return diff / values.length;
            }
        },
        {
            id: 101,
            label: 'Moving Average Standard Deviation',
            description: 'The square root of the average deviation from the moving average with Bessel\'s correction.',
            execute: function (values, movingAverages) {
                if (values.length < 1)
                    return NaN;

                var diffSquareSum = 0;
                for (var i = 1; i < values.length; i++) {
                    var diff = (values[i] - movingAverages[i]);
                    diffSquareSum += diff * diff;
                }

                return Math.sqrt(diffSquareSum / (values.length - 1));
            }
        },
    ];
})();

if (typeof module != 'undefined') {
    for (var key in Statistics)
        module.exports[key] = Statistics[key];
}
