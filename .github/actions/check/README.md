# check Action

Check if we passed the comparison or not.

This is just here to isolate it. 
Essentially, we are given the package of plots generated by the validate action
and we check if there are any plots in the `fail` directory.
If there are, we print an error message and list the plots that failed the comparison.

We also want to isolate this check because the validate action is used
to check **and** to produce new golden histograms. We might want to keep the new golden
histograms despite the fact that they disagree with the current golden (perhaps we found and patched a bug).

## Inputs

- `plots` : **required**
  - Full path to package of plots generated by validate action

