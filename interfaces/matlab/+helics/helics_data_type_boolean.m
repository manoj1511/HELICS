function v = helics_data_type_boolean()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 19);
  end
  v = vInitialized;
end
