#!/usr/bin/python

from LDMX.Framework import ldmxcfg

trigScintDigis = ldmxcfg.Producer("trigScintDigis", "ldmx::TrigScintDigiProducer")

trigScintDigis.parameters["meanNoise"] = 0.02
trigScintDigis.parameters["number_of_strips"] = 62
trigScintDigis.parameters["number_of_arrays"] = 1
trigScintDigis.parameters["mev_per_mip"] = 0.4
trigScintDigis.parameters["pe_per_mip"] = 10.
trigScintDigis.parameters["input_collection"]="TriggerPadUpSimHits"
trigScintDigis.parameters["output_collection"]="trigScintDigis"

trigScintDigisDn = ldmxcfg.Producer("trigScintDigis", "ldmx::TrigScintDigiProducer")

trigScintDigisDn.parameters["meanNoise"] = 0.02
trigScintDigisDn.parameters["number_of_strips"] = 62
trigScintDigisDn.parameters["number_of_arrays"] = 1
trigScintDigisDn.parameters["mev_per_mip"] = 0.4
trigScintDigisDn.parameters["pe_per_mip"] = 10.
trigScintDigisDn.parameters["input_collection"]="TriggerPadDownSimHits"
trigScintDigisDn.parameters["output_collection"]="trigScintDigisDn"

trigScintDigisTag = ldmxcfg.Producer("trigScintDigis", "ldmx::TrigScintDigiProducer")

trigScintDigisTag.parameters["meanNoise"] = 0.02
trigScintDigisTag.parameters["number_of_strips"] = 62
trigScintDigisTag.parameters["number_of_arrays"] = 1
trigScintDigisTag.parameters["mev_per_mip"] = 0.4
trigScintDigisTag.parameters["pe_per_mip"] = 10.
trigScintDigisTag.parameters["input_collection"]="TriggerPadTaggerSimHits"
trigScintDigisTag.parameters["output_collection"]="trigScintDigisTag"
