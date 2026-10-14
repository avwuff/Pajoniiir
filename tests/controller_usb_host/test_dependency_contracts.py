"""Run the real ancestry CMake transform and scoped Git-source checkout policy."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root=Path(__file__).resolve().parents[2]
cmake=shutil.which('cmake')
if not cmake and os.name=='nt':cmake=str(sorted(Path('C:/Espressif/tools/cmake').glob('*/bin/cmake.exe'))[-1])
ninja=shutil.which('ninja')
if not ninja and os.name=='nt':ninja=str(sorted(Path('C:/Espressif/tools/ninja').glob('*/ninja.exe'))[-1])
git=shutil.which('git');assert cmake and ninja and git
fixture=(Path(__file__).parent/'fixtures/hub_parent_registration.c').read_text()
managed=root/'firmware/main-deck-jc1060/managed_components/usb/src/hub.c'
if managed.exists():assert fixture[fixture.index('static esp_err_t dev_tree_node_new('):].strip() in managed.read_text()
with tempfile.TemporaryDirectory() as td:
    folder=Path(td);(folder/'src').mkdir()
    (folder/'src/hub.c').write_text(fixture)
    (folder/'src/ext_hub.c').write_text('/* ext_hub_dev->constant.dev_hdl */\n')
    hook=(root/'firmware/p4-core/cmake/apply_espressif_usb_parent_info_patch.cmake').as_posix()
    (folder/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.22)
project(ancestry NONE)
if(NOT HUB_INPUT)
 set(HUB_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/hub.c")
endif()
add_custom_target(usb_stub SOURCES "${HUB_INPUT}" "${CMAKE_CURRENT_SOURCE_DIR}/src/ext_hub.c")
function(idf_component_get_property output component property)
 if(property STREQUAL "COMPONENT_DIR")
  set(${output} "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
 else()
  set(${output} usb_stub PARENT_SCOPE)
 endif()
endfunction()
include("'''+hook+'''")
get_target_property(sources usb_stub SOURCES)
file(WRITE "${CMAKE_BINARY_DIR}/selected.txt" "${sources}")
''')
    args=[cmake,'-G','Ninja','-DCMAKE_MAKE_PROGRAM='+ninja,'-S',str(folder),'-B',str(folder/'build')]
    p=subprocess.run(args,capture_output=True,text=True);assert p.returncode==0,p.stdout+p.stderr
    selected=(folder/'build/selected.txt').read_text().split(';')
    assert selected==[(folder/'build/pajoniiir_usb_parent/hub.c').as_posix(),(folder/'build/pajoniiir_usb_parent/ext_hub.c').as_posix()]
    generated=(folder/'build/pajoniiir_usb_parent/hub.c').read_text()
    assert '.parent_dev_hdl = parent_dev_hdl,' in generated and '.parent_dev_hdl = NULL,' not in generated
    assert 'heap_caps_free(dev_tree_node);\n            return ESP_ERR_NOT_FOUND;' in generated
    prior=folder/'prior';prior.mkdir()
    (prior/'hub.c').write_text(fixture+'\n/* earlier indexed root recovery remains selected */\n')
    p=subprocess.run(args+['-DHUB_INPUT='+(prior/'hub.c').as_posix()],capture_output=True,text=True)
    assert p.returncode==0,p.stdout+p.stderr
    assert 'earlier indexed root recovery remains selected' in (folder/'build/pajoniiir_usb_parent/hub.c').read_text()
    (folder/'src/hub.c').write_text(fixture.replace('.parent_dev_hdl = NULL,','.parent_dev_hdl = changed,'))
    p=subprocess.run(args+['-DHUB_INPUT='+(folder/'src/hub.c').as_posix()],capture_output=True,text=True);assert p.returncode!=0 and 'parent metadata changed' in p.stderr
    # A resolver clone must use the lock's CRLF bytes even if the caller requested LF.
    source=folder/'git-source';source.mkdir()
    env=dict(os.environ);env.update(GIT_CONFIG_COUNT='1',GIT_CONFIG_KEY_0='core.autocrlf',GIT_CONFIG_VALUE_0='false')
    subprocess.run([git,'init','-q',str(source)],check=True,env=env)
    (source/'sample.c').write_bytes(b'first\nsecond\n')
    subprocess.run([git,'-C',str(source),'add','sample.c'],check=True,env=env)
    subprocess.run([git,'-C',str(source),'-c','user.name=host-test','-c','user.email=host@test.invalid','commit','-qm','fixture'],check=True,env=env)
    clone=folder/'checkout';policy=(root/'firmware/p4-core/cmake/dependency_checkout.cmake').as_posix()
    script=folder/'checkout.cmake';script.write_text('''include("'''+policy+'''")
execute_process(COMMAND "'''+Path(git).as_posix()+'''" clone --quiet "'''+source.as_posix()+'''" "'''+clone.as_posix()+'''" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
 message(FATAL_ERROR "clone failed")
endif()
if(NOT "$ENV{GIT_CONFIG_COUNT}" STREQUAL "2")
 message(FATAL_ERROR "inherited Git settings were not preserved")
endif()
''')
    subprocess.run([cmake,'-P',str(script)],check=True,env=env)
    assert (clone/'sample.c').read_bytes()==b'first\r\nsecond\r\n'
print('PASS real CMake ancestry source selection, upstream drift rejection, and scoped CRLF dependency clone with inherited settings')
