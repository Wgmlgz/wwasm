import React from "react";
import "./App.css";

import WCanvas from "./wwasm/react/canvas.jsx";

class App extends React.Component {
  render() {
    return (
      <div className="App">
        <WCanvas />
      </div>
    );
  }
}

export default App;
