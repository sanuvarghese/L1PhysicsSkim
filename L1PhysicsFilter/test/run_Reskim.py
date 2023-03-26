# to apply further prescales on L1Skim and obtain re-skimmed raw files. based on Andrea's code

import FWCore.ParameterSet.Config as cms

from Configuration.Eras.Era_Run3_cff import Run3

process = cms.Process("HLT3")


# Options                                                                                                                                    
nEvents= -1           # number of events to process                                                                                          
outputName="L1.root"  # output file name                                                                                                     
                                                                                           
from list_cff_Skim import inputFileNames
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(inputFileNames),
    lumisToProcess = cms.untracked.VLuminosityBlockRange("362439:35-362439:220"),
    inputCommands = cms.untracked.vstring('keep *')
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( nEvents )
)


from EventFilter.L1TRawToDigi.l1tRawToDigi_cfi import l1tRawToDigi as _l1tRawToDigi
process.l1tRawToDigi = _l1tRawToDigi.clone(
    FedIds = [ 1404 ],
    Setup = 'stage2::GTSetup',
    InputLabel = "rawDataCollector"
)
process.triggerTypePhysics = cms.EDFilter("HLTTriggerTypeFilter",
    SelectedTriggerType = cms.int32(1)
)

from L1Trigger.L1TGlobal.l1tGlobalPrescaler_cfi import l1tGlobalPrescaler as _l1tGlobalPrescaler
process.l1tGlobalPrescaler = _l1tGlobalPrescaler.clone(
    l1tResults = 'l1tRawToDigi',                                                                                                     
)

process.l1tGlobalPrescaler.l1tPrescales[34] = 0
process.l1tGlobalPrescaler.l1tPrescales[273] = 0
process.l1tGlobalPrescaler.l1tPrescales[274] = 0        # apply the prescales you want. Index corresponds to the bit used in the L1 Skim.
process.l1tGlobalPrescaler.l1tPrescales[275] = 0
process.l1tGlobalPrescaler.l1tPrescales[276] = 0        
process.l1tGlobalPrescaler.l1tPrescales[277] = 0
process.l1tGlobalPrescaler.l1tPrescales[78] = 0
process.l1tGlobalPrescaler.l1tPrescales[64] = 0

process.Skim = cms.Path(process.triggerTypePhysics+process.l1tRawToDigi + process.l1tGlobalPrescaler)

process.hltOutputTriggerResults = cms.OutputModule( "PoolOutputModule",
        fileName = cms.untracked.string(outputName),
            SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring("Skim")
        ),
        outputCommands = cms.untracked.vstring("drop *",
                                               "keep *")
                                                  )


process.l1filteroutput = cms.EndPath(process.hltOutputTriggerResults)

process.schedule = cms.Schedule(process.Skim,process.l1filteroutput)







