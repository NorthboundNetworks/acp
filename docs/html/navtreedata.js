/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "ACP - Autonomous Command Protocol", "index.html", [
    [ "Autonomous Command Protocol v0.2 — Specification", "md_acp__comm__spec__v0-2.html", [
      [ "Purpose", "md_acp__comm__spec__v0-2.html#autotoc_md1", null ],
      [ "Goals", "md_acp__comm__spec__v0-2.html#autotoc_md3", null ],
      [ "Protocol Frame Structure (v0.2)", "md_acp__comm__spec__v0-2.html#autotoc_md5", null ],
      [ "Message Types", "md_acp__comm__spec__v0-2.html#autotoc_md7", [
        [ "GCS → Drone Commands", "md_acp__comm__spec__v0-2.html#autotoc_md8", null ],
        [ "Drone → GCS Telemetry / Events", "md_acp__comm__spec__v0-2.html#autotoc_md9", null ],
        [ "Responses", "md_acp__comm__spec__v0-2.html#autotoc_md10", null ],
        [ "Response Structs", "md_acp__comm__spec__v0-2.html#autotoc_md11", null ]
      ] ],
      [ "Example Structs (Packed C Representations)", "md_acp__comm__spec__v0-2.html#autotoc_md13", [
        [ "<span class=\"tt\">cmd_control_t</span>", "md_acp__comm__spec__v0-2.html#autotoc_md14", null ],
        [ "<span class=\"tt\">tlm_status_t</span>", "md_acp__comm__spec__v0-2.html#autotoc_md15", null ],
        [ "<span class=\"tt\">tlm_audit_t</span>", "md_acp__comm__spec__v0-2.html#autotoc_md16", null ]
      ] ],
      [ "Safety &amp; Mediation Requirements", "md_acp__comm__spec__v0-2.html#autotoc_md18", [
        [ "Error Codes", "md_acp__comm__spec__v0-2.html#autotoc_md19", null ]
      ] ],
      [ "Extensibility &amp; Future Features", "md_acp__comm__spec__v0-2.html#autotoc_md21", null ],
      [ "Implementation Notes", "md_acp__comm__spec__v0-2.html#autotoc_md23", null ],
      [ "Success Criteria", "md_acp__comm__spec__v0-2.html#autotoc_md25", null ],
      [ "Out of Scope", "md_acp__comm__spec__v0-2.html#autotoc_md27", null ]
    ] ],
    [ "Autonomous Command Protocol v0.3 — Specification", "md_acp__comm__spec__v0-3.html", [
      [ "Purpose", "md_acp__comm__spec__v0-3.html#autotoc_md29", null ],
      [ "Goals", "md_acp__comm__spec__v0-3.html#autotoc_md31", null ],
      [ "Protocol Frame Structure (v0.3)", "md_acp__comm__spec__v0-3.html#autotoc_md33", null ],
      [ "Message Types", "md_acp__comm__spec__v0-3.html#autotoc_md35", [
        [ "GCS → Drone Commands", "md_acp__comm__spec__v0-3.html#autotoc_md36", null ],
        [ "Drone → GCS Telemetry / Events", "md_acp__comm__spec__v0-3.html#autotoc_md37", null ],
        [ "Responses", "md_acp__comm__spec__v0-3.html#autotoc_md38", null ],
        [ "Response Structs", "md_acp__comm__spec__v0-3.html#autotoc_md39", null ]
      ] ],
      [ "Example Structs (Packed C Representations)", "md_acp__comm__spec__v0-3.html#autotoc_md41", [
        [ "<span class=\"tt\">cmd_control_t</span>", "md_acp__comm__spec__v0-3.html#autotoc_md42", null ],
        [ "<span class=\"tt\">tlm_status_t</span>", "md_acp__comm__spec__v0-3.html#autotoc_md43", null ],
        [ "<span class=\"tt\">tlm_audit_t</span>", "md_acp__comm__spec__v0-3.html#autotoc_md44", null ]
      ] ],
      [ "Safety &amp; Mediation Requirements", "md_acp__comm__spec__v0-3.html#autotoc_md46", [
        [ "Error Codes", "md_acp__comm__spec__v0-3.html#autotoc_md47", null ]
      ] ],
      [ "Extensibility &amp; Future Features", "md_acp__comm__spec__v0-3.html#autotoc_md49", null ],
      [ "Implementation Notes", "md_acp__comm__spec__v0-3.html#autotoc_md51", null ],
      [ "Success Criteria", "md_acp__comm__spec__v0-3.html#autotoc_md53", null ],
      [ "Out of Scope", "md_acp__comm__spec__v0-3.html#autotoc_md55", null ]
    ] ],
    [ "ACP Library Build Guide", "md__b_u_i_l_d.html", [
      [ "Table of Contents", "md__b_u_i_l_d.html#autotoc_md57", null ],
      [ "Quick Start", "md__b_u_i_l_d.html#autotoc_md58", [
        [ "Prerequisites", "md__b_u_i_l_d.html#autotoc_md59", null ],
        [ "Fast Build (CMake)", "md__b_u_i_l_d.html#autotoc_md60", null ],
        [ "Fast Build (Make)", "md__b_u_i_l_d.html#autotoc_md61", null ]
      ] ],
      [ "Build Systems", "md__b_u_i_l_d.html#autotoc_md62", [
        [ "CMake (Recommended)", "md__b_u_i_l_d.html#autotoc_md63", null ],
        [ "Make", "md__b_u_i_l_d.html#autotoc_md64", null ]
      ] ],
      [ "Platform Support", "md__b_u_i_l_d.html#autotoc_md65", [
        [ "Linux (x86_64, ARM64)", "md__b_u_i_l_d.html#autotoc_md66", null ],
        [ "macOS (Intel, Apple Silicon)", "md__b_u_i_l_d.html#autotoc_md67", null ],
        [ "Windows (MinGW, MSVC)", "md__b_u_i_l_d.html#autotoc_md68", null ],
        [ "Embedded Systems", "md__b_u_i_l_d.html#autotoc_md69", null ]
      ] ],
      [ "Build Options", "md__b_u_i_l_d.html#autotoc_md70", [
        [ "CMake Options", "md__b_u_i_l_d.html#autotoc_md71", null ],
        [ "Make Options", "md__b_u_i_l_d.html#autotoc_md72", null ]
      ] ],
      [ "Installation", "md__b_u_i_l_d.html#autotoc_md73", [
        [ "System-wide Installation", "md__b_u_i_l_d.html#autotoc_md74", null ],
        [ "Package Manager Integration", "md__b_u_i_l_d.html#autotoc_md75", null ]
      ] ],
      [ "Integration", "md__b_u_i_l_d.html#autotoc_md76", [
        [ "Using Static Library", "md__b_u_i_l_d.html#autotoc_md77", null ],
        [ "Using Shared Library", "md__b_u_i_l_d.html#autotoc_md78", null ],
        [ "Code Example", "md__b_u_i_l_d.html#autotoc_md79", null ]
      ] ],
      [ "Cross-Compilation", "md__b_u_i_l_d.html#autotoc_md80", [
        [ "ARM Linux (aarch64)", "md__b_u_i_l_d.html#autotoc_md81", null ],
        [ "ARM Embedded (Cortex-M)", "md__b_u_i_l_d.html#autotoc_md82", null ],
        [ "Windows from Linux (MinGW)", "md__b_u_i_l_d.html#autotoc_md83", null ]
      ] ],
      [ "Troubleshooting", "md__b_u_i_l_d.html#autotoc_md84", [
        [ "Common Build Issues", "md__b_u_i_l_d.html#autotoc_md85", null ],
        [ "Platform-Specific Issues", "md__b_u_i_l_d.html#autotoc_md86", null ],
        [ "Debug Builds", "md__b_u_i_l_d.html#autotoc_md87", null ],
        [ "Getting Help", "md__b_u_i_l_d.html#autotoc_md88", null ]
      ] ]
    ] ],
    [ "Specification Quality Checklist: ACP Protocol Project Requirements and Deliverables", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html", [
      [ "Content Quality", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md105", null ],
      [ "Requirement Completeness", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md106", null ],
      [ "Feature Readiness", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md107", null ],
      [ "Notes", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md108", null ],
      [ "Validation Results (Iteration 1)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md110", null ],
      [ "Validation Results (Iteration 2)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2checklists_2requirements.html#autotoc_md112", null ]
    ] ],
    [ "C API Contract — ACP v0.3", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html", [
      [ "Conventions", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md114", null ],
      [ "Types (conceptual)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md115", null ],
      [ "Functions", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md116", [
        [ "Framing and Integrity", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md117", null ],
        [ "Session Management", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md118", null ],
        [ "Keystore Interface (platform shim)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md119", null ]
      ] ],
      [ "Error Semantics", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md120", null ],
      [ "Versioning", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2contracts_2c-api.html#autotoc_md121", null ]
    ] ],
    [ "Data Model — ACP v0.3", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html", [
      [ "Entities", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md123", [
        [ "ACP Frame", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md124", null ],
        [ "Session", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md125", null ],
        [ "Keystore", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md126", null ],
        [ "Platform Shims", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md127", null ]
      ] ],
      [ "State Transitions (Session)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md128", null ],
      [ "Error Codes (excerpt)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2data-model.html#autotoc_md129", null ]
    ] ],
    [ "Implementation Plan: ACP Protocol Project Requirements and Deliverables", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html", [
      [ "Summary", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md131", null ],
      [ "Technical Context", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md132", null ],
      [ "Constitution Check", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md133", null ],
      [ "Project Structure", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md134", [
        [ "Documentation (this feature)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md135", null ],
        [ "Source Code (repository root)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md136", null ]
      ] ],
      [ "Complexity Tracking", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2plan.html#autotoc_md137", null ]
    ] ],
    [ "Quickstart — ACP Reference C Library", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html", [
      [ "Prerequisites", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md139", null ],
      [ "Build", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md140", null ],
      [ "Minimal usage", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md141", null ],
      [ "Tests", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md142", null ],
      [ "Platform shims", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md143", null ],
      [ "Notes", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2quickstart.html#autotoc_md144", null ]
    ] ],
    [ "Phase 0 Research — ACP Protocol Implementation Details", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html", [
      [ "Framing and Integrity", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md146", null ],
      [ "Authentication and Replay Protection", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md147", null ],
      [ "Frame Header and Endianness", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md148", null ],
      [ "Payload Constraints and Errors", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md149", null ],
      [ "Keystore", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md150", null ],
      [ "Build and Toolchains", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md151", null ],
      [ "Testing", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md152", null ],
      [ "Open Questions (resolved here)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2research.html#autotoc_md153", null ]
    ] ],
    [ "Feature Specification: ACP Protocol Project Requirements and Deliverables", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html", [
      [ "Overview", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md155", null ],
      [ "Clarifications", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md156", [
        [ "Session 2025-10-27", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md157", null ]
      ] ],
      [ "User Scenarios &amp; Testing (mandatory)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md158", [
        [ "User Story 1 - Encode/Decode Telemetry Frame (Priority: P1)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md159", null ],
        [ "User Story 2 - Authenticate Session With HMAC (Priority: P1)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md161", null ],
        [ "User Story 3 - Cross-Platform Build and Link (Priority: P2)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md163", null ],
        [ "User Story 4 - Platform Abstraction Fallbacks (Priority: P3)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md165", null ],
        [ "Edge Cases", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md166", null ]
      ] ],
      [ "Requirements (mandatory)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md167", [
        [ "Functional Requirements", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md168", null ],
        [ "Key Entities (data-oriented)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md169", null ]
      ] ],
      [ "Success Criteria (mandatory)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md170", [
        [ "Measurable Outcomes", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md171", null ]
      ] ],
      [ "Assumptions and Dependencies", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2spec.html#autotoc_md172", null ]
    ] ],
    [ "Implementation Tasks — ACP Protocol Project Requirements and Deliverables", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html", [
      [ "Phase 1 — Setup (project initialization)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md174", null ],
      [ "Phase 2 — Foundational (blocking prerequisites)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md175", null ],
      [ "Phase 3 — User Story 1 (P1): Encode/Decode Telemetry Frame", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md176", null ],
      [ "Phase 4 — User Story 2 (P1): Authenticate Session With HMAC", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md177", null ],
      [ "Phase 5 — User Story 3 (P2): Cross-Platform Build and Link", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md178", null ],
      [ "Phase 5A — Extended Build System (completed)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md179", null ],
      [ "Phase 6 — User Story 4 (P3): Platform Abstraction Fallbacks", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md180", null ],
      [ "Final Phase — Polish &amp; Cross-Cutting", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md181", null ],
      [ "Dependencies (story completion order)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md182", null ],
      [ "Parallel execution examples", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md183", null ],
      [ "Implementation Status Update", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md184", null ],
      [ "Implementation Summary (October 27, 2025)", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md185", [
        [ "<b>Core Protocol Features (100% Complete)</b>", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md186", null ],
        [ "<b>Build System &amp; Developer Experience (100% Complete)</b>", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md187", null ],
        [ "<b>Testing &amp; Validation (100% Complete)</b>", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md188", null ],
        [ "<b>Production Readiness Criteria Met</b>", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md189", null ]
      ] ],
      [ "Implementation strategy", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md190", null ],
      [ "Validation", "md__2_users_2paulzanna_2_github_2acp_2specs_2001-acp-protocol-spec_2tasks.html#autotoc_md191", null ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"_c_make_c_compiler_id_8c_source.html",
"acp__platform__posix_8c.html#a2c83600265e90db7a27efb252def9f06",
"md__b_u_i_l_d.html#autotoc_md71"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';