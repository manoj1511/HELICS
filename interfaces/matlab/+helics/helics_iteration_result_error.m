function v = helics_iteration_result_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 93);
  end
  v = vInitialized;
end
