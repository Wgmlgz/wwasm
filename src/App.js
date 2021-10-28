import React from "react";
import WCanvas from "./wwasm/react/canvas.jsx";
import frog from "./wwasm/cpp/images/frog.bmp";
import smol_frog from "./wwasm/cpp/images/smol_frog.jpg";

class App extends React.Component {
  render() {
    return (
      <div className="app">
        <div className="hidden">
          <img id="frog" src={frog} />
          <img id="smol frog" src={smol_frog} />
        </div>
        <div className="cavnas">
          <WCanvas canvas_id="js" render="js" />
        </div>
      </div>
    );
  }
}

export default App;
