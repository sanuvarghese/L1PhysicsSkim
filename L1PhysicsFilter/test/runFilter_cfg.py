# Import HLT configuration #
from data import *

# STEAM Customization #

# Options
nEvents=-1           # number of events to process
#switchL1PS=False       # apply L1 PS ratios to switch to tighter column
#columnL1PS=1           # choose the tighter column ( 0 <=> tightest )
outputName="L1.root"  # output file name

# Input
from list_cff import inputFileNames
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(inputFileNames),
    inputCommands = cms.untracked.vstring('keep *')
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32( nEvents )
)

# L1 customizations
#from HLTrigger.Configuration.common import *
#import itertools

def insert_modules_after(process, target, *modules):
    "Add the `modules` after the `target` in any Sequence, Paths or EndPath that contains the latter."                                                      
    for sequence in itertools.chain(
        process._Process__sequences.itervalues(),
        process._Process__paths.itervalues(),
        process._Process__endpaths.itervalues()
    ):                                                                                                                                                      
        try:
            position = sequence.index ( target )
        except ValueError:
            continue
        else:
            for module in reversed(modules):
                sequence.insert(position+1, module)

process.L1PhysicsFilter = cms.EDFilter("L1PhysicsFilter",
                                       hltProcess=cms.string("HLT2")
                                       stageL1Trigger=cms.uint32(2)
                                       )

process.l1filter_step = cms.Path(process.L1PhysicsFilter)
process.schedule.append(process.l1filter_step)

process.hltOutputTriggerResults = cms.OutputModule( "PoolOutputModule",
        fileName = cms.untracked.string(outputName),
        SelectEvents = cms.untracked.PSet( 
        SelectEvents = cms.vstring("l1filter_step")
        ),
        outputCommands = cms.untracked.vstring('drop *',
                                       'keep *')
                                        
      )
process.l1filteroutput = cms.EndPath(process.hltOutputTriggerResults)
process.schedule.append(process.l1filteroutput)
# Customisation from command line

# Add early deletion of temporary data products to reduce peak memory need
#from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
#process = customiseEarlyDelete(process)

#process.hltOutputTriggerResults = cms.OutputModule( "PoolOutputModule",
#         fileName = cms.untracked.string( "FilteredOutput.root" ),
#      )

# End adding early deletion
#print process.schedule
