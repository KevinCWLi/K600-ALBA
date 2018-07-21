//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: exampleB4a.cc 75215 2013-10-29 16:07:06Z gcosmo $
//
/// \file exampleB4a.cc
/// \brief Main program of the B4a example

#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "G4UImanager.hh"
#include "G4UIcommand.hh"
#include "FTFP_BERT.hh"
#include "QGSP_BERT.hh"
#include "QGSP_BERT_HP.hh"

#include "G4PhysListFactory.hh"
#include "G4RadioactiveDecayPhysics.hh"

#include "Randomize.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

#ifdef G4UI_USE
#include "G4UIExecutive.hh"
#endif

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

namespace {
    void PrintUsage() {
        G4cerr << " Usage: " << G4endl;
        G4cerr << " exampleB4a [-m macro ] [-u UIsession] [-t nThreads]" << G4endl;
        G4cerr << "   note: -t option is available only for multi-threaded mode."
        << G4endl;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc,char** argv)
{
    // Evaluate arguments
    //
    if ( argc > 7 ) {
        PrintUsage();
        return 1;
    }
    
    G4String macro;
    G4String session;
#ifdef G4MULTITHREADED
    G4int nThreads = 4;
#endif
    for ( G4int i=1; i<argc; i=i+2 ) {
        if      ( G4String(argv[i]) == "-m" ) macro = argv[i+1];
        else if ( G4String(argv[i]) == "-u" ) session = argv[i+1];
#ifdef G4MULTITHREADED
        else if ( G4String(argv[i]) == "-t" ) {
            nThreads = G4UIcommand::ConvertToInt(argv[i+1]);
        }
#endif
        else {
            PrintUsage();
            return 1;
        }
    }
    
    
    
    // Choose the Random engine
    //
    //G4Random::setTheEngine(new CLHEP::RanecuEngine);
    
    CLHEP::RanluxEngine defaultEngine( 1234567, 4 );
    G4Random::setTheEngine( &defaultEngine );
    G4int seed = time( NULL );
    G4Random::setTheSeed( seed );
    
    // Construct the default run manager
    //
#ifdef G4MULTITHREADED
    G4MTRunManager * runManager = new G4MTRunManager;
    runManager->SetNumberOfThreads(nThreads);
    
    /*
     if ( nThreads > 0 ) {
     runManager->SetNumberOfThreads(4);
     }
     */
#else
    G4RunManager * runManager = new G4RunManager;
#endif
    
    // Set mandatory initialization classes
    //
    DetectorConstruction* detConstruction = new DetectorConstruction();
    runManager->SetUserInitialization(detConstruction);
    
    /*
     G4VModularPhysicsList* physicsList = new QGSP_BERT;
     runManager->SetUserInitialization(physicsList);
     */
    
    ////////////////////////////////////////////////////////////////////
    //      Initialising the Physics List with Radioactive Decay
    ////////////////////////////////////////////////////////////////////
    
    G4PhysListFactory factory;
    G4VModularPhysicsList* phys = 0;
    G4String physName = "QGSP_BERT";
    // reference PhysicsList via its name
    phys = factory.GetReferencePhysList(physName);
    phys->RegisterPhysics(new G4RadioactiveDecayPhysics());
    runManager->SetUserInitialization(phys);
    
    
    ActionInitialization* actionInitialization
    = new ActionInitialization(detConstruction);
    runManager->SetUserInitialization(actionInitialization);
    
    // Initialize G4 kernel
    //
    runManager->Initialize();
    
#ifdef G4VIS_USE
    // Initialize visualization
    G4VisManager* visManager = new G4VisExecutive;
    // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
    // G4VisManager* visManager = new G4VisExecutive("Quiet");
    visManager->Initialize();
#endif
    
    // Get the pointer to the User Interface manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    
    if ( macro.size() ) {
        // batch mode
        G4String command = "/control/execute ";
        UImanager->ApplyCommand(command+macro);
    }
    else  {
        // interactive mode : define UI session
#ifdef G4UI_USE
        G4UIExecutive* ui = new G4UIExecutive(argc, argv, session);
#ifdef G4VIS_USE
        UImanager->ApplyCommand("/control/execute init_vis.mac");
#else
        UImanager->ApplyCommand("/control/execute init.mac");
#endif
        if (ui->IsGUI())
            UImanager->ApplyCommand("/control/execute gui.mac");
        ui->SessionStart();
        delete ui;
#endif
    }
    
    // Job termination
    // Free the store: user actions, physics_list and detector_description are
    // owned and deleted by the run manager, so they should not be deleted
    // in the main() program !
    
#ifdef G4VIS_USE
    delete visManager;
#endif
    delete runManager;
    
    return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
