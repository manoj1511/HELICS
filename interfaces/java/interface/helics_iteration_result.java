/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 4.0.0
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.helics;

public final class helics_iteration_result {
  public final static helics_iteration_result helics_iteration_result_next_step = new helics_iteration_result("helics_iteration_result_next_step");
  public final static helics_iteration_result helics_iteration_result_error = new helics_iteration_result("helics_iteration_result_error");
  public final static helics_iteration_result helics_iteration_result_halted = new helics_iteration_result("helics_iteration_result_halted");
  public final static helics_iteration_result helics_iteration_result_iterating = new helics_iteration_result("helics_iteration_result_iterating");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static helics_iteration_result swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + helics_iteration_result.class + " with value " + swigValue);
  }

  private helics_iteration_result(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private helics_iteration_result(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private helics_iteration_result(String swigName, helics_iteration_result swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static helics_iteration_result[] swigValues = { helics_iteration_result_next_step, helics_iteration_result_error, helics_iteration_result_halted, helics_iteration_result_iterating };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

