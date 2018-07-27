Simulated Annealing Algorithm for Floorplan
                                                                   
 Operations:                                                        
 - Accepts data files containing:                                   
   area size, area constraints, orientation constraint      
                                                                      
   Example invocation:                                                
                                                                      
   Text mode:                                                         
   - floorplan test.dat                                               
   - e.g. context of test.dat                                         
                                                                      
     #numbers 3          // number of modules                         
     #p 0.5              // p <= q                                    
     #q 2                // r <= s                                    
     16 0.5 0.5 0        // Area, r, s, orientation                   
     10 1   2   1                                                     
     16 1   1   1                                                     
     #end                                                             
