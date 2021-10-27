import React from "react";
import WCanvas from "./wwasm/react/canvas.jsx";

class App extends React.Component {
  render() {
    return (
      <div className="app">
        <div className="cavnas">
          <WCanvas canvas_id="main_canvas" />
        </div>
      </div>
    );
  }
}

export default App;
